
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CCDPlayerState.generated.h"

UCLASS()
class CCD_API ACCDPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 유저 이름
	UPROPERTY(ReplicatedUsing = OnRep_CustomName, BlueprintReadOnly, Category = "PlayerInfo")
	FString CustomName = TEXT("None");
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerInfo")
	FString SteamName = TEXT("");

	// 플레이어 사망 여부
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerInfo")
	bool bIsDead = false;

	// 남은 리스폰 시간
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerInfo")
	float RemainingRespawnTime = 0.0f;

	UFUNCTION()
	void OnRep_CustomName() const;
};
