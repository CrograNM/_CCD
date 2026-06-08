
#include "Actor/CCD_KillZone.h"

#include "Component/ProgressComponent.h"
#include "Components/BoxComponent.h"


ACCD_KillZone::ACCD_KillZone()
{
	PrimaryActorTick.bCanEverTick = false;
	
	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	RootComponent = TriggerZone;

	// 충돌 프로필 설정 (쿼리 전용, overlap 전용으로 설정하면 효율적입니다)
	TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
}

void ACCD_KillZone::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ACCD_KillZone::OnOverlapBegin);
}

void ACCD_KillZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 유효한 액터이고 자기 자신(킬존)이 아닐 때만 검사
	if (OtherActor && OtherActor != this)
	{
		if (UProgressComponent* ProgressComp = OtherActor->FindComponentByClass<UProgressComponent>())
		{
			if (ProgressComp->ProgressValue > 0.0f)
			{
				// 3. 조건 만족 시 액터 파괴
				OtherActor->Destroy();
			}
		}
	}
}

