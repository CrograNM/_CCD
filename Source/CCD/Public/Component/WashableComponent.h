
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WashableComponent.generated.h"

UENUM(BlueprintType)
enum class ECCD_WashableType : uint8
{
	EWT_Blood		UMETA(DisplayName = "Blood"),		// 핏자국
	EWT_Excrement	UMETA(DisplayName = "Excrement")	// 배설물
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CCD_API UWashableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWashableComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	
	// 오염물질 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Washable")
	ECCD_WashableType WashableType = ECCD_WashableType::EWT_Blood;
	
	UPROPERTY(ReplicatedUsing = OnRep_WashHealth, EditAnywhere, Category = "Status")
	float WashHealth = 100.f;
	UFUNCTION()
	void OnRep_WashHealth();
	
	// 같은 액터에 있는 점수 컴포넌트 참조
	class UProgressComponent* ProgressComp;

public:	
	UFUNCTION(BlueprintCallable, Category = "Washable")
	ECCD_WashableType GetWashableType() const { return WashableType; }
	
	UFUNCTION(BlueprintCallable, Category = "Washable")
	float getWashHealthRatio() const { return WashHealth / 100.f; }
	
	UFUNCTION(BlueprintCallable, Category = "Washable")
	void TakeWashDamage(float DamageAmount);
};