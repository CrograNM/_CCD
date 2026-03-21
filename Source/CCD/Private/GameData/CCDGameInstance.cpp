
#include "GameData/CCDGameInstance.h"

#include "OnlineSubsystemUtils.h"
#include "GameData/CCDSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UCCDGameInstance::Init()
{
	Super::Init();
	UserProfileName = GetSavedName();
	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UCCDGameInstance::OnCreateSessionComplete);
	DestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UCCDGameInstance::OnDestroySessionComplete);
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
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(); 
	
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
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
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
		}
	}
}

void UCCDGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
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
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		}
	}

	// 세션이 성공적으로 파괴되었거나 실패했더라도 메인 메뉴로 이동한다
	UGameplayStatics::OpenLevel(GetWorld(), FName(*MainMenuPath));
}
