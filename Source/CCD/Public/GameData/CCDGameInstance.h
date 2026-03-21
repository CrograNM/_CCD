
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

	UFUNCTION(BlueprintCallable)
	FString GetSavedName() const;

	/** --- Session --- **/
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void HostSession(FString RoomName, bool bIsLAN, FString Path);

	UFUNCTION(BlueprintPure, Category = "Multiplayer")
	FString GetRoomNameFromSearchResult(FBlueprintSessionResult SearchResult) const;
	
	UFUNCTION(BlueprintPure, Category = "Steam")
	FString GetSteamNameIfAvailable() const;
	
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	
private:
	FString UserProfileName;
	FString SaveSlotName = TEXT("UserProfile");
	FString MapPath = TEXT("");
	
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
};
