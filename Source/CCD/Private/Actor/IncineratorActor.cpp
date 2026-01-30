
#include "Actor/IncineratorActor.h"
#include "Components/BoxComponent.h"
#include "Component/BurnableComponent.h"

AIncineratorActor::AIncineratorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	BurnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("BurnArea"));
	BurnArea->SetupAttachment(RootComponent);
	
	// 서버에서만 대미지 판정을 하도록 설정
	BurnArea->SetCollisionProfileName(TEXT("Trigger"));
}

void AIncineratorActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		BurnArea->OnComponentBeginOverlap.AddDynamic(this, &AIncineratorActor::OnBurnAreaBeginOverlap);
		BurnArea->OnComponentEndOverlap.AddDynamic(this, &AIncineratorActor::OnBurnAreaEndOverlap);
	}
}

void AIncineratorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!HasAuthority()) return;

	// 영역 안의 모든 물체에 초당 대미지 부여
	for (int32 i = OverlappingBurnables.Num() - 1; i >= 0; --i)
	{
		if (OverlappingBurnables[i] && OverlappingBurnables[i]->GetOwner())
		{
			float Damage = DamagePerSecond * DeltaTime;
			OverlappingBurnables[i]->TakeBurnDamage(Damage);
		}
		else
		{
			OverlappingBurnables.RemoveAt(i);
		}
	}
}

void AIncineratorActor::OnBurnAreaBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (UBurnableComponent* BurnComp = OtherActor->FindComponentByClass<UBurnableComponent>())
		{
			OverlappingBurnables.AddUnique(BurnComp);
		}
	}
}

void AIncineratorActor::OnBurnAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		if (UBurnableComponent* BurnComp = OtherActor->FindComponentByClass<UBurnableComponent>())
		{
			OverlappingBurnables.Remove(BurnComp);
		}
	}
}

void AIncineratorActor::Interact_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Warning, TEXT("IncineratorActor::Interact_Implementation"));
}
