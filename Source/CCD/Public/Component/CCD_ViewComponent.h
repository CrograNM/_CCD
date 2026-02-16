
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CCD_ViewComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CCD_API UCCD_ViewComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCCD_ViewComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 외부 인터페이스
	void ToggleView();
	void ApplyViewMode(bool bFirstPerson);
	
	UFUNCTION(BlueprintCallable)
	bool GetIsFirstPerson() const { return bIsFirstPerson; }
	
protected:
	virtual void BeginPlay() override;
	
	/** --- 서버 권한 로직 (RPC) --- */
	UFUNCTION(Server, Reliable)
	void Server_ToggleView(bool bNewIsFirstPerson);

	UFUNCTION(Server, Unreliable)
	void Server_SetFirstPersonCameraRotation(FRotator NewRotation);

	UFUNCTION(Server, Unreliable)
	void Server_SetControlRotation(FRotator NewRotation);

private:
	/** --- 상태 및 컴포넌트 참조 --- */
	UPROPERTY(ReplicatedUsing = OnRep_IsFirstPerson)
	bool bIsFirstPerson = true;

	UPROPERTY(Replicated)
	FRotator Rep_FirstPersonCameraRotation;
	
	UFUNCTION()
	void OnRep_IsFirstPerson();

	UPROPERTY() TObjectPtr<class ACCDCharacter> OwnerCharacter;
	UPROPERTY() TObjectPtr<UCameraComponent> FollowCamera;
	UPROPERTY() TObjectPtr<UCameraComponent> FirstPersonCamera;
	UPROPERTY() TObjectPtr<USpringArmComponent> CameraBoom;

	// 회전 동기화용 변수
	FRotator LastSentRotation;
	const float RotationThreshold = 0.1f;
};
