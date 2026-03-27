
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProgressComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CCD_API UProgressComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UProgressComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 매니저를 매번 찾지 않도록 저장
	class AProgressManager* ProgressManager;
	
public:	
	UPROPERTY(ReplicatedUsing = OnRep_ProgressValue, EditAnywhere, BlueprintReadWrite, Category = "Progress")
	float ProgressValue = 0.0f;
	UFUNCTION()
	void OnRep_ProgressValue();
	
	void UpdateProgressValue(float NewValue);
	
	// 액터가 소각되거나 대걸레질이 완료되었을 때 호출
	void Notify_ProgressOver();
	
private:
	bool bIsTaskFinished = false;
};
