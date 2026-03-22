
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "FindSessionsCallbackProxy.h"
#include "CCDGameInstance.generated.h"

UCLASS()
class CCD_API UCCDGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void SaveCustomName(FString NewName);

	/** --- Getters --- **/
	UFUNCTION(BlueprintCallable)
	FString GetSavedName() const;
	UFUNCTION(BlueprintPure, Category = "Multiplayer")
	FString GetRoomNameFromSearchResult(FBlueprintSessionResult SearchResult) const;
	UFUNCTION(BlueprintPure, Category = "Steam")
	FString GetSteamNameIfAvailable() const;
	
	/** --- Session --- **/
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void HostSession(FString RoomName, bool bIsLAN, FString Path);
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void LeaveSession();
	
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	
private:
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	
	FString UserProfileName;
	FString SaveSlotName = TEXT("UserProfile");
	FString LobbyMapPath = TEXT("/Game/_CCD/Maps/Lobby");
	FString MainMenuPath = TEXT("/Game/_CCD/Maps/LV_MainMenu");
	
	// Delegates for session management
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
};
