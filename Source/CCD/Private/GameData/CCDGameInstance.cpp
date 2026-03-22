
#include "GameData/CCDGameInstance.h"

#include "OnlineSubsystemUtils.h"
#include "GameData/CCDSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

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
			UGameplayStatics::OpenLevel(GetWorld(), FName(*MainMenuPath));
			UE_LOG(LogTemp, Warning, TEXT("No active session found. Returning to main menu."));
		}
	}
	else 
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName(*MainMenuPath));
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
		GetWorld()->ServerTravel(LobbyMapPath + TEXT("?listen"));
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
	UGameplayStatics::OpenLevel(GetWorld(), FName(*MainMenuPath));
}

void UCCDGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
	const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("Network Disconnected: %s. Cleaning up local session..."), *ErrorString);
	
	LeaveSession();
}
