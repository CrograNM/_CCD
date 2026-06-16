
#include "Actor/WasteActor_Base.h"

#include "Component/BurnableComponent.h"
#include "Component/ProgressComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Hearing.h"

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
	float ImpulseSize = NormalImpulse.Size();

	if (ImpulseSize < HitSoundThreshold) return;
	if (GetWorld()->GetTimeSeconds() - LastSoundTime < HitSoundCoolDown) return;
	
	float TargetVolume = FMath::GetMappedRangeValueClamped(FVector2D(HitSoundThreshold, HitSoundThreshold + 2000.f), FVector2D(0.2f, 0.8f), ImpulseSize);
	UGameplayStatics::PlaySoundAtLocation(this, HitSound, Hit.ImpactPoint, TargetVolume, 1, 0, HitAttenuation);
	
	if (HasAuthority()) 
	{
		float Loudness = TargetVolume * 0.5f; 

		AActor* NoiseAgent = GetOwner() ? GetOwner() : Cast<AActor>(GetInstigator());
		if (!NoiseAgent)
		{
			NoiseAgent = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		}

		if (NoiseAgent)
		{
			UAISense_Hearing::ReportNoiseEvent(NoiseAgent, Hit.ImpactPoint, Loudness, Cast<APawn>(NoiseAgent));
		}

#if WITH_EDITOR
		if (GetWorld())
		{
			float BaseHearingRange = 2500.0f;
			float SoundRadius = BaseHearingRange * Loudness;

			DrawDebugSphere(
				GetWorld(),
				Hit.ImpactPoint,
				SoundRadius,
				16,
				FColor::Yellow,
				false,
				0.5f,
				0,
				1.5f
			);
		}
#endif
	}

	LastSoundTime = GetWorld()->GetTimeSeconds();
}