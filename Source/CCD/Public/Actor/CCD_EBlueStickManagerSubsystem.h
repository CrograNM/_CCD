
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CCD_EBlueStickManagerSubsystem.generated.h"

class ACCD_EBlueStick;

UCLASS()
class CCD_API UCCD_EBlueStickManagerSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	// --- 생명주기 함수 ---
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// --- FTickableGameObject 인터페이스 ---
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UCCD_EBlueStickManagerSubsystem, STATGROUP_Tickables); }
	
	// 블루스틱이 스폰될 때 등록
	void RegisterStick(ACCD_EBlueStick* InStick);
	
	// 블루스틱이 사라질 때 해제
	void UnregisterStick(ACCD_EBlueStick* InStick);

	
	
private:
	UPROPERTY()
	TArray<TWeakObjectPtr<ACCD_EBlueStick>> ActiveSticks;

	UPROPERTY()
	TObjectPtr<class UMaterialParameterCollection> UVLightMPC;

	const int32 MAX_STICK_COUNT = 3; // MPC에 만든 변수 개수와 일치시켜야 함
};
