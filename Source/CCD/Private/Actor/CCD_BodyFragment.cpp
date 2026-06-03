
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
		if (HasAuthority())
		{	
			RepSkeletalMesh = MeshComp->GetSkeletalMeshAsset();
			OnRep_SkeletalMesh();
		}
		MeshComp->OnComponentHit.AddDynamic(this, &ACCD_BodyFragment::OnMeshHit);
	}
}

void ACCD_BodyFragment::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// 기본 유효성 검사
	if (!HitSound || DecalStainActorClasses.Num() == 0 || !HitEffect) return;
    
	// 쿨다운 체크
	if (GetWorld()->GetTimeSeconds() - LastHitTime < HitCoolDown) return;
	
	// 충격량 계산 및 임계값 체크
	const float ImpulseSize = NormalImpulse.Size();
	
	// ----- 사운드 재생 충격량 필터링 -----
	if (ImpulseSize < HitSoundThreshold) return;
	
	// 사운드 재생
	const float TargetVolume = FMath::GetMappedRangeValueClamped(FVector2D(HitSoundThreshold, HitSoundThreshold + 2000.f), FVector2D(0.2f, 0.8f), ImpulseSize);
	UGameplayStatics::PlaySoundAtLocation(this, HitSound, Hit.ImpactPoint, TargetVolume, 1, 0, HitSoundAttenuation);
	LastHitTime = GetWorld()->GetTimeSeconds(); 
	
	// ----- VFX & 데칼 충격량 필터링 -----
	if (ImpulseSize < HitEffectThreshold) return;
	
	// 충격량에 따른 스케일 계산 (MinStainSize ~ MaxStainSize)
    const float TargetScale = FMath::GetMappedRangeValueClamped(
    		FVector2D(MinImpulseForStainSize, MaxImpulseForStainSize), 
    		FVector2D(MinStainSize, MaxStainSize), 
    		ImpulseSize);
    			
	// VFX 생성
	FRotator SpawnRot = Hit.ImpactNormal.Rotation();
	SpawnRot.Pitch -= 90.0f;
	
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, Hit.Location, SpawnRot, FVector(TargetScale * 2.0f));
    
	// 핏자국 생성
	if (!HasAuthority()) return;
	if (CurrentStainThreshold < MaxStainThreshold)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const int32 RandomIndex = FMath::RandHelper(DecalStainActorClasses.Num());
		if (const TSubclassOf<ADecal_StainActor_Base> SelectedDecalClass = DecalStainActorClasses[RandomIndex])
		{
			ADecal_StainActor_Base* SpawnedDecal = GetWorld()->SpawnActor<ADecal_StainActor_Base>(
				SelectedDecalClass, 
				Hit.Location + Hit.ImpactNormal * 1.5f,
				SpawnRot, 
				SpawnParams);

			if (SpawnedDecal)
			{
				// 데칼 액터의 스케일 적용
				SpawnedDecal->SetActorScale3D(FVector(TargetScale));
            
				// 충격량 총량 업데이트 (최대값으로 클램핑)
				float ClampedImpulse = ImpulseSize;
				if (ClampedImpulse > MaxImpulseForStainSize) 
					ClampedImpulse = MaxImpulseForStainSize;
				CurrentStainThreshold += ClampedImpulse;
			}
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
		MeshComp->SetCollisionResponseToChannel(ECC_DecalSurface, ECR_Ignore); // 데칼 표면 검사와 충돌 무시
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetNotifyRigidBodyCollision(true);
		MeshComp->SetUseCCD(true);
		
		// MeshComp->bReplicatePhysicsToAutonomousProxy = true;
		// SetPhysicsReplicationMode(EPhysicsReplicationMode::Resimulation);
	}
}