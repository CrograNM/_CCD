
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "FindSessionsCallbackProxy.h"
#include "CCDGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCustomFindSessionsComplete, const TArray<FBlueprintSessionResult>&, Results, bool, bWasSuccessful);

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
	void FindSessionsCustom(int32 MaxResults, bool bIsLAN, bool bUseLobbies);
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void HostSession(FString RoomName, bool bIsLAN, FString Path);
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void LeaveSession();
	void LeaveSessionForEnding();
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void CleanupLocalSession();
	
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	
	void OnFindSessionsComplete(bool bWasSuccessful);
	UPROPERTY(BlueprintAssignable, Category = "Multiplayer")
	FOnCustomFindSessionsComplete OnCustomFindSessionsComplete;
	
	UFUNCTION(BlueprintPure, Category = "Steam")
	bool IsSteamActive() const;
	
	UPROPERTY(BlueprintReadWrite, Category = "GameFlow")
	bool bHasPlayedPrologue = false;
	
	void TransitionLevel(FString NextLevelPath);
	
private:
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	
	FString UserProfileName;
	FString SaveSlotName = TEXT("UserProfile");
	FString LobbyMapPath = TEXT("/Game/Maps/TUWorld");
	FString MainMenuPath = TEXT("/Game/Maps/Title");
	FString EndingMapPath = TEXT("/Game/Maps/Ending");
	
	// Delegates for session management
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	
	// 세션 검색 설정을 저장할 포인터
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
};
