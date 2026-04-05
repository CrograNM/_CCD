#pragma once

#include "CoreMinimal.h"
#include <GameFramework/Actor.h>
#include "CCD_EquipActor_Base.generated.h"

class ACCDCharacter;

UCLASS(Abstract)
class CCD_API ACCD_EquipActor_Base : public AActor
{
	GENERATED_BODY()

public:
	ACCD_EquipActor_Base();
	virtual void OnRep_AttachmentReplication() override;
	
	// 장비 사용 시 호출
	virtual void ExecuteAction() {}
	
	// 애니메이션 노티파이 등에서 정밀한 타이밍에 호출할 로직
	virtual void OnActionNotify() {}

	// 장비 활성화/비활성화 (예: 장비 교체 시)
	virtual void SetEquipmentActive(bool bActive);

	// 장비 초기화 (소유자 캐릭터 참조 설정)
	void InitializeEquipment(ACCDCharacter* InOwner);
	
protected:
	virtual void BeginPlay() override;

	bool IsOwnedByLocalPlayer() const;
	
	// 소유자 캐릭터 참조
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<ACCDCharacter> OwnerCharacter;

	// 현재 활성화 상태
	bool bIsActive = false;

};
