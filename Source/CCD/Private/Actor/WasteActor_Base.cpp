
#include "Actor/WasteActor_Base.h"

#include "Component/BurnableComponent.h"
#include "Component/ProgressComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AWasteActor_Base::AWasteActor_Base()
{
	PrimaryActorTick.bCanEverTick = false;

	// 네트워크 복제 설정
	bReplicates = true;
	bNetLoadOnClient = true;
	AActor::SetReplicateMovement(true);
	bAlwaysRelevant = true;
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetIsReplicated(true);
	MeshComp->SetNotifyRigidBodyCollision(true);
	
	// 컴포넌트 생성 및 포함
	BurnableComp = CreateDefaultSubobject<UBurnableComponent>(TEXT("BurnableComp"));
	BurnableComp->SetIsReplicated(true);
	
	ProgressComp = CreateDefaultSubobject<UProgressComponent>(TEXT("ProgressComp"));
	ProgressComp->SetIsReplicated(true);
}

void AWasteActor_Base::BeginPlay()
{
	Super::BeginPlay();
	
	// 충돌 이벤트 바인딩
	if (MeshComp)
	{
		MeshComp->OnComponentHit.AddDynamic(this, &AWasteActor_Base::OnMeshHit);
	}
}

void AWasteActor_Base::UpdatePhysicsReplicates(bool inReplicates)
{
	bReplicates = inReplicates;
	//MeshComp->SetEnableGravity(inReplicates);
	//MeshComp->SetSimulatePhysics(inReplicates);
}

void AWasteActor_Base::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HitSound) return;
	
	// 충격 강도 계산
	float ImpulseSize = NormalImpulse.Size();

	// 최소 충격량보다 작거나, 쿨타임(예: 0.5초)이 지나지 않았다면 무시
	if (ImpulseSize < HitSoundThreshold) return;
	if (GetWorld()->GetTimeSeconds() - LastSoundTime < HitSoundCoolDown) return;

	// 충격 강도에 따라 볼륨 조절 - 0.2 ~ 1.0 사이의 볼륨으로 클램프
	float TargetVolume = FMath::GetMappedRangeValueClamped(FVector2D(HitSoundThreshold, HitSoundThreshold + 2000.f), FVector2D(0.2f, 0.8f), ImpulseSize);

	// 충돌 지점에서 소리 재생
	UGameplayStatics::PlaySoundAtLocation(this, HitSound, Hit.ImpactPoint, TargetVolume, 1, 0, HitAttenuation);
	
	LastSoundTime = GetWorld()->GetTimeSeconds();
}