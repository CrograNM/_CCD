
#include "Actor/CCD_BodyFragment.h"

#include "NiagaraFunctionLibrary.h"
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
	// 기본 유효성 검사
	if (!HitSound || !DecalStainActorClass) return;
    
	// 충격량 계산 및 임계값 체크
	float ImpulseSize = NormalImpulse.Size();
	
	if (ImpulseSize < HitSoundThreshold) return;

	// 쿨다운 체크
	if (GetWorld()->GetTimeSeconds() - LastHitTime < HitCoolDown) return;
	
	UE_LOG(LogTemp, Warning, TEXT("%s - 충돌 발생 ImpulseSize = %f"), *GetName(), ImpulseSize);
    
	// 사운드 재생
	float TargetVolume = FMath::GetMappedRangeValueClamped(FVector2D(HitSoundThreshold, HitSoundThreshold + 2000.f), FVector2D(0.2f, 0.8f), ImpulseSize);
	UGameplayStatics::PlaySoundAtLocation(this, HitSound, Hit.ImpactPoint, TargetVolume, 1, 0, HitSoundAttenuation);
    
	if (HitEffect)
	{
		FRotator SpawnRot = Hit.ImpactNormal.Rotation();
		SpawnRot.Pitch -= 90.0f;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, Hit.Location, SpawnRot);
	}
	
	LastHitTime = GetWorld()->GetTimeSeconds();
    
	// 핏자국 생성
	if (ImpulseSize >= HitEffectThreshold && CurrentStainCount < MaxStainCount)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		FRotator SpawnRot = Hit.ImpactNormal.Rotation();
		SpawnRot.Pitch -= 90.0f;
       
		if (GetWorld()->SpawnActor<ADecal_StainActor_Base>(DecalStainActorClass, Hit.Location, SpawnRot, SpawnParams))
		{
			CurrentStainCount++;
		}
	}
	
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
		MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetNotifyRigidBodyCollision(true);
	}
}