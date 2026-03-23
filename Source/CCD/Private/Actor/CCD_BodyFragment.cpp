
#include "Actor/CCD_BodyFragment.h"

#include "Actor/Decal_StainActor_Base.h"
#include "Component/BurnableComponent.h"
#include "Component/ProgressComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ACCD_BodyFragment::ACCD_BodyFragment()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	bNetLoadOnClient = true;
	AActor::SetReplicateMovement(true);
	bAlwaysRelevant = true;
	
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetIsReplicated(true);
	
	// 초기에는 물리 시뮬레이션과 충돌 비활성화 (스폰 이후 시점에 활성화하기)
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	
	MeshComp->SetNotifyRigidBodyCollision(true);
	MeshComp->SetGenerateOverlapEvents(true);
	
	BurnableComp = CreateDefaultSubobject<UBurnableComponent>(TEXT("BurnableComp"));
	BurnableComp->SetIsReplicated(true);
	
	ProgressComp = CreateDefaultSubobject<UProgressComponent>(TEXT("ProgressComp"));
	ProgressComp->SetIsReplicated(true);
	ProgressComp->ProgressValue = 5.0f;
}

void ACCD_BodyFragment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCD_BodyFragment, RepSkeletalMesh);
}

void ACCD_BodyFragment::BeginPlay()
{
	Super::BeginPlay();
	
	if (MeshComp)
	{
		MeshComp->OnComponentHit.AddDynamic(this, &ACCD_BodyFragment::OnMeshHit);
	}
}

void ACCD_BodyFragment::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("%s - OnMeshHit: ImpulseSize = %f"), *GetName(), NormalImpulse.Size());
	if (!HitSound) return;
	
	// 충격 강도 계산
	float ImpulseSize = NormalImpulse.Size();

	// 최소 충격량보다 작거나, 쿨타임(예: 0.5초)이 지나지 않았다면 무시
	if (ImpulseSize < HitThreshold) return;
	if (GetWorld()->GetTimeSeconds() - LastSoundTime < HitCoolDown) return;

	// 충격 강도에 따라 볼륨 조절 - 0.2 ~ 1.0 사이의 볼륨으로 클램프
	float TargetVolume = FMath::GetMappedRangeValueClamped(FVector2D(HitThreshold, HitThreshold + 2000.f), FVector2D(0.2f, 0.8f), ImpulseSize);

	// 충돌 지점에서 소리 재생
	UGameplayStatics::PlaySoundAtLocation(this, HitSound, Hit.ImpactPoint, TargetVolume, 1, 0, HitAttenuation);
	
	LastSoundTime = GetWorld()->GetTimeSeconds();
	
	// 충돌 지점에서 데칼 스폰, 이펙트 재생
	FHitResult HitResult = Hit;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FRotator SpawnRot = HitResult.ImpactNormal.Rotation();
	SpawnRot.Pitch -= 90.0f;
	GetWorld()->SpawnActor<ADecal_StainActor_Base>(DecalStainActorClass, HitResult.Location, SpawnRot, SpawnParams);
}

void ACCD_BodyFragment::InitFragment(USkeletalMesh* InMesh, FVector Impulse)
{
	if (HasAuthority() && InMesh)
	{
		RepSkeletalMesh = InMesh;
		OnRep_SkeletalMesh();
		MeshComp->AddImpulse(Impulse, NAME_None, true);
	}
}

void ACCD_BodyFragment::OnRep_SkeletalMesh()
{
	if (RepSkeletalMesh)
	{
		MeshComp->SetSkeletalMesh(RepSkeletalMesh);
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));
	}
}