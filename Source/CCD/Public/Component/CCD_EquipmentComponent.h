
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

UENUM(BlueprintType)
enum class ECCD_EquipmentState : uint8
{
	EES_Hands   UMETA(DisplayName = "Hands"),   // 맨손 (Physics Handle)
	EES_Scanner UMETA(DisplayName = "Scanner"), // 탐지장치
	EES_Mop     UMETA(DisplayName = "Mop")		// 대걸레
};

#include "CCD_EquipmentComponent.generated.h"

class ACCDCharacter;
class UWidgetComponent;
class UScannerWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CCD_API UCCD_EquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCCD_EquipmentComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	/** --- 외부 인터페이스 --- */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SwitchEquipment(ECCD_EquipmentState NewState) { Server_SetEquipmentState(NewState); }
	
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void HandleEquipNotify();
	
	void ProceedToEquip(ECCD_EquipmentState NewState);
	
	/** --- Getter --- */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	ECCD_EquipmentState GetEquipmentState() const { return EquipmentState; }
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	ECCD_EquipmentState GetPendingEquipmentState() const { return PendingEquipmentState; }
	
	// Mop
	UPROPERTY(ReplicatedUsing=OnRep_Pollution)
	float MopPollution_Blood = 0.0f;
	
	UPROPERTY(ReplicatedUsing=OnRep_Pollution)
	float MopPollution_Excrement = 0.0f;
	
	UFUNCTION()
	void OnRep_Pollution();
	
	void UpdateMopMeshPollution();
	
	// Scanner 
	UFUNCTION(BlueprintCallable, Category = "E_Scanner")
	float GetScanActorDistance() const;
	const float MaxScanDistance = 1000.f; // 탐지 최대 거리
	void ScannerUpdate(float Distance) const;
	
protected:
	virtual void BeginPlay() override;
	
	/** --- 장비 상태 및 메시 --- */
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentState)
	ECCD_EquipmentState EquipmentState = ECCD_EquipmentState::EES_Hands;
	
	ECCD_EquipmentState PendingEquipmentState = ECCD_EquipmentState::EES_Hands;

	UPROPERTY() 
	TObjectPtr<UStaticMeshComponent> MopMesh;
	
	UPROPERTY() 
	TObjectPtr<UStaticMeshComponent> ScannerMesh;
	
	// 스캐너 메쉬에 부착될 3D 위젯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scanner UI")
	TObjectPtr<UWidgetComponent> ScannerWidgetComp;
	
	// 위젯 내부의 실제 UI 클래스 참조
	UPROPERTY()
	TObjectPtr<UScannerWidget> ScannerWidget;
	
	/** --- 네트워크 RPC --- */
	UFUNCTION(Server, Reliable)
	void Server_SetEquipmentState(ECCD_EquipmentState NewState);

	UFUNCTION()
	void OnRep_EquipmentState(ECCD_EquipmentState PreviousState);

	/** --- 내부 로직 --- */
	void HandleEquipmentEffects(ECCD_EquipmentState NewState);

private:
	// 소유자 캐릭터 참조
	UPROPERTY() class ACCDCharacter* OwnerCharacter {};
};
