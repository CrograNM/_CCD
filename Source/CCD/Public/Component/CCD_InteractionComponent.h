
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CCD_InteractionComponent.generated.h"

class UPhysicsHandleComponent;
class UCameraComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CCD_API UCCD_InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCCD_InteractionComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 캐릭터가 호출할 인터페이스
	void PerformInteract();
	
protected:
	virtual void BeginPlay() override;
	
	/** --- 서버 권한 로직 (RPC) --- */
	UFUNCTION(Server, Reliable)
	void Server_PerformInteract();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_GrabObject(UPrimitiveComponent* ComponentToGrab, FVector GrabLocation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ReleaseObject();
	
private:
	/** --- 내부 컴포넌트 및 변수 --- */
	UPROPERTY()
	TObjectPtr<UPhysicsHandleComponent> PhysicsHandle;

	UPROPERTY(Replicated)
	TObjectPtr<UPrimitiveComponent> GrabbedComponent;
	
	UPROPERTY()
	FRotator GrabRelativeRotation;

	UPROPERTY(EditAnywhere, Category = "Design")
	float InteractRange = 300.f;

	// 헬퍼 함수
	void PhysicsHandleUpdate(float DeltaTime);
	void GrabObject_Impl(UPrimitiveComponent* ComponentToGrab, FVector GrabLocation);
	void ReleaseObject_Impl();

	UPROPERTY()
	TObjectPtr<class ACCDCharacter> OwnerCharacter;
};
