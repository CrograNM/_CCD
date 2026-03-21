
#include "GameData/CCDGameInstance.h"

#include "OnlineSubsystemUtils.h"
#include "GameData/CCDSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UCCDGameInstance::Init()
{
	Super::Init();
	UserProfileName = GetSavedName();
	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UCCDGameInstance::OnCreateSessionComplete);
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

void UCCDGameInstance::HostSession(FString RoomName, bool bIsLAN, FString Path)
{
	if (Path == TEXT(""))
	{
		MapPath = TEXT("/Game/_CCD/Maps/Lobby");
	}
	else MapPath = Path;
	
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
		GetWorld()->ServerTravel(MapPath + TEXT("?listen"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to Create Session!"));
	}
}
