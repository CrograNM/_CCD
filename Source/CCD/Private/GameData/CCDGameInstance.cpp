
#include "GameData/CCDGameInstance.h"

#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystemUtils.h"
#include "GameData/CCDSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameData/CCDGameMode.h"

const FString UniqueBuildID = TEXT("ContainmentCleanupDetail_v0.0.1");

void UCCDGameInstance::Init()
{
	Super::Init();
	UserProfileName = GetSavedName();
	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UCCDGameInstance::OnCreateSessionComplete);
	DestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UCCDGameInstance::OnDestroySessionComplete);

	// 네트워크 실패 이벤트 바인딩
	if (GEngine)
	{	
		GEngine->OnNetworkFailure().AddUObject(this, &UCCDGameInstance::HandleNetworkFailure);
	}
}

void UCCDGameInstance::SaveCustomName(FString NewName)
{
	UserProfileName = NewName;
	if (UCCDSaveGame* SaveInstance = Cast<UCCDSaveGame>(UGameplayStatics::CreateSaveGameObject(UCCDSaveGame::StaticClass())))
	{
		SaveInstance->SavedPlayerName = NewName;
		UGameplayStatics::SaveGameToSlot(SaveInstance, SaveSlotName, 0);
	}
}
FString UCCDGameInstance::GetSavedName() const
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		if (UCCDSaveGame* LoadedGame = Cast<UCCDSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)))
		{
			return LoadedGame->SavedPlayerName;
		}
	}
	return TEXT("None");
}
FString UCCDGameInstance::GetRoomNameFromSearchResult(FBlueprintSessionResult SearchResult) const
{
	if (FString FoundRoomName; SearchResult.OnlineResult.Session.SessionSettings.Get(FName(TEXT("RoomName")), FoundRoomName))
	{
		return FoundRoomName;
	}
	return SearchResult.OnlineResult.Session.OwningUserName;
}
FString UCCDGameInstance::GetSteamNameIfAvailable() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	
	if (Subsystem && Subsystem->GetSubsystemName() == FName(TEXT("Steam")))
	{
		IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
		if (Identity.IsValid())
		{
			// 로그인된 0번 로컬 유저의 닉네임을 가져옴
			return Identity->GetPlayerNickname(0);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Subsystem Name: %s"), 
		Subsystem ? *Subsystem->GetSubsystemName().ToString() : TEXT("None"));
	return TEXT("");
}

void UCCDGameInstance::FindSessionsCustom(int32 MaxResults, bool bIsLAN, bool bUseLobbies)
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem)
	{
		OnCustomFindSessionsComplete.Broadcast(TArray<FBlueprintSessionResult>(), false);
		return;
	}

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		
		SessionSearch->MaxSearchResults = MaxResults;
		SessionSearch->bIsLanQuery = bIsLAN;
		
		if(bUseLobbies)
		{
			SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
		}
		SessionSearch->QuerySettings.Set(FName(TEXT("BUILD_ID")), UniqueBuildID, EOnlineComparisonOp::Equals);
		
		// 완료 콜백 등록
		FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
			FOnFindSessionsCompleteDelegate::CreateUObject(this, &UCCDGameInstance::OnFindSessionsComplete));

		UE_LOG(LogTemp, Warning, TEXT("Starting Custom Session Search... (Max: %d)"), MaxResults);
		
		if (!SessionInterface->FindSessions(0, SessionSearch.ToSharedRef()))
		{
			// 검색 시작 실패 시 즉시 빈 결과 반환
			SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
			OnCustomFindSessionsComplete.Broadcast(TArray<FBlueprintSessionResult>(), false);
			UE_LOG(LogTemp, Error, TEXT("Failed to start session search!"));
		}
	}
}

void UCCDGameInstance::HostSession(FString RoomName, bool bIsLAN, FString Path)
{
	// 로비 맵 경로가 전달되면 업데이트, 그렇지 않으면 기본값 사용
	if (Path != TEXT("")) LobbyMapPath = Path; 
	
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem) return;

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
		
		FOnlineSessionSettings SessionSettings;
		SessionSettings.NumPublicConnections = 3;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.bIsLANMatch = bIsLAN;
		
		SessionSettings.bUsesPresence = !bIsLAN;
		SessionSettings.bUseLobbiesIfAvailable = !bIsLAN;
		SessionSettings.bAllowJoinViaPresence = !bIsLAN;
		
		SessionSettings.Set(FName(TEXT("RoomName")), RoomName, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings.Set(FName(TEXT("BUILD_ID")), UniqueBuildID, EOnlineDataAdvertisementType::ViaOnlineService);
		
		SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
	}
}
void UCCDGameInstance::LeaveSession()
{
	UE_LOG(LogTemp, Warning, TEXT("Attempting to leave session..."));

	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem) return;

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		if (SessionInterface->GetNamedSession(NAME_GameSession))
		{
			DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
			SessionInterface->DestroySession(NAME_GameSession);
		}
		else
		{
			// UGameplayStatics::OpenLevel(GetWorld(), FName(*MainMenuPath));
			if (ACCDGameMode* GM = Cast<ACCDGameMode>(GetWorld()->GetAuthGameMode()))
			{
				GM->TransitionToLevel(*MainMenuPath);
			}
			UE_LOG(LogTemp, Warning, TEXT("No active session found. Returning to main menu."));
		}
	}
	else 
	{
		// UGameplayStatics::OpenLevel(GetWorld(), FName(*MainMenuPath));
		if (ACCDGameMode* GM = Cast<ACCDGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->TransitionToLevel(*MainMenuPath);
		}
		UE_LOG(LogTemp, Warning, TEXT("Session Interface invalid. Cannot leave session cleanly."));
	}
}

void UCCDGameInstance::CleanupLocalSession()
{
	UE_LOG(LogTemp, Warning, TEXT("Cleaning up leftover local session..."));
	
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// 로컬에 'GameSession'이라는 이름의 세션이 남아있다면 강제 파괴
			if (SessionInterface->GetNamedSession(NAME_GameSession))
			{
				// 맵 이동 콜백 등을 기다리지 않고 즉시 메모리에서 날려버립니다.
				SessionInterface->DestroySession(NAME_GameSession);
				UE_LOG(LogTemp, Warning, TEXT("Leftover Local Session Destroyed Cleanly."));
			}
			else 
			{
				UE_LOG(LogTemp, Warning, TEXT("No leftover local session found."));
			}
		}
		else 
		{
			UE_LOG(LogTemp, Warning, TEXT("Session Interface invalid. Cannot clean up local session."));
		}
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("No Online Subsystem found. Cannot clean up local session."));
	}
}

void UCCDGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		}
	}

	if (bWasSuccessful)
	{
		// GetWorld()->ServerTravel(LobbyMapPath + TEXT("?listen"));
		if (ACCDGameMode* GM = Cast<ACCDGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->TransitionToLevel(LobbyMapPath + TEXT("?listen"));
		}
		else 
		{
			UE_LOG(LogTemp, Error, TEXT("CCDGameMode not found. Transition Failed"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to Create Session!"));
	}
}
void UCCDGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		}
		else 
		{
			UE_LOG(LogTemp, Warning, TEXT("Session Interface invalid. Cannot clear destroy session delegate."));
		}
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("No Online Subsystem found. Cannot clear destroy session delegate."));
	}
	
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to Destroy Session! Returning to main menu anyway."));
	}
	// UGameplayStatics::OpenLevel(GetWorld(), FName(*MainMenuPath));
	if (ACCDGameMode* GM = Cast<ACCDGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->TransitionToLevel(*MainMenuPath);
	}
}

void UCCDGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		}
	}
	
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		// 필터링 전, 스팀이 던져준 '순수 결과 개수' 확인
		UE_LOG(LogTemp, Warning, TEXT("Steam returned %d raw results before filtering."), SessionSearch->SearchResults.Num());
        
		for (auto& Result : SessionSearch->SearchResults)
		{
			// 검색된 방의 소유자 이름이라도 찍어보기
			UE_LOG(LogTemp, Log, TEXT("Found Session by: %s"), *Result.Session.OwningUserName);
		}
	}

	TArray<FBlueprintSessionResult> BlueprintResults;

	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Search Complete. Found %d Sessions."), SessionSearch->SearchResults.Num());

		for (auto& Result : SessionSearch->SearchResults)
		{
			// 기본 Find Sessions 노드에서 사용하는 구조체로 변환
			FBlueprintSessionResult BPResult;
			BPResult.OnlineResult = Result;
			BlueprintResults.Add(BPResult);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Session Search Failed."));
	}

	// 블루프린트로 결과 방송
	OnCustomFindSessionsComplete.Broadcast(BlueprintResults, bWasSuccessful);
}

bool UCCDGameInstance::IsSteamActive() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	return (Subsystem && Subsystem->GetSubsystemName() == FName(TEXT("Steam")));
}

void UCCDGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
                                            const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("Network Disconnected: %s. Cleaning up local session..."), *ErrorString);
	
	LeaveSession();
}
