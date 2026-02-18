
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
class UCCD_ScannerComponent;
class ACCD_EquipActor_Base;

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
	
	// Use Equipment
	void ExcuteActiveEquipment() const;
	
protected:
	virtual void BeginPlay() override;
	
	/** --- 장비 상태 및 메시 --- */
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentState)
	ECCD_EquipmentState EquipmentState = ECCD_EquipmentState::EES_Hands;
	
	ECCD_EquipmentState PendingEquipmentState = ECCD_EquipmentState::EES_Hands;

	UPROPERTY() 
	TObjectPtr<UStaticMeshComponent> MopMesh;
	
	/** --- 네트워크 RPC --- */
	UFUNCTION(Server, Reliable)
	void Server_SetEquipmentState(ECCD_EquipmentState NewState);

	UFUNCTION()
	void OnRep_EquipmentState(ECCD_EquipmentState PreviousState);

	/** --- 내부 로직 --- */
	void HandleEquipmentEffects(ECCD_EquipmentState NewState);
	
	/** --- 장비 자동 스폰 및 저장소 --- */
	// 에디터 디테일 패널에서 어떤 상태(Mop, Scanner)에 어떤 클래스를 스폰할지 지정합니다.
	UPROPERTY(EditAnywhere, Category = "Equipment | Setup")
	TMap<ECCD_EquipmentState, TSubclassOf<ACCD_EquipActor_Base>> ToolClassMap;

	// 생성된 실제 액터들을 저장해두는 저장소입니다.
	UPROPERTY()
	TMap<ECCD_EquipmentState, TObjectPtr<ACCD_EquipActor_Base>> SpawnedToolMap;

private:
	void InitializeEquipment(); // 초기화 함수
	
	UPROPERTY()
	TObjectPtr<ACCDCharacter> OwnerCharacter;
};
