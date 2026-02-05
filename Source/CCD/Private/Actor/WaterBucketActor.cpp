
#include "Actor/WaterBucketActor.h"

#include "Component/ProgressComponent.h"
#include "Net/UnrealNetwork.h"


AWaterBucketActor::AWaterBucketActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 물 양동이 액터는 초기 진행도를 0으로 설정
	ProgressComp->ProgressValue = 0.0f;
	
	WaterMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMeshComp"));
	WaterMeshComp->SetupAttachment(RootComponent); 
	WaterMeshComp->SetIsReplicated(true);
}

void AWaterBucketActor::BeginPlay()
{
	Super::BeginPlay();
	
	// 머티리얼 인스턴스 생성 (블루프린트에서 StaticMesh에 할당된 재질 인덱스 0번 가정)
	if (WaterMeshComp)
	{
		WaterMaterial = WaterMeshComp->CreateAndSetMaterialInstanceDynamic(0);
	}
}

void AWaterBucketActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 일정 이상의 각도에 도달하면 물이 쏟아지는 효과
	if (WaterMaterial && HasAuthority())
	{
		FRotator Rotation = GetActorRotation();
		if (FMath::Abs(Rotation.Pitch) > 60.0f || FMath::Abs(Rotation.Roll) > 60.0f)
			SpillWater();
	}
}

void AWaterBucketActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AWaterBucketActor, Pollution_Blood);
	DOREPLIFETIME(AWaterBucketActor, Pollution_Excrement);
}

bool AWaterBucketActor::WashMop(float& InBloodAmount, float& InExcrementAmount)
{
	return false;
}

void AWaterBucketActor::OnRep_Pollution()
{
}

void AWaterBucketActor::UpdateWaterColor()
{
}

void AWaterBucketActor::SpillWater()
{
	UE_LOG(LogTemp, Warning, TEXT("Spill Water!"));
	WaterMeshComp ->SetVisibility(false);
	WaterMaterial = nullptr;
}
