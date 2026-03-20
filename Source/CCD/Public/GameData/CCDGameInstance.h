
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
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

private:
	FString UserProfileName;
	FString SaveSlotName = TEXT("UserProfile");
};
