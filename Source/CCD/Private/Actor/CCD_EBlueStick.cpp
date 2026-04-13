
#include "Actor/CCD_EBlueStick.h"

#include "Component/ProgressComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"


ACCD_EBlueStick::ACCD_EBlueStick()
{
	PrimaryActorTick.bCanEverTick = false;
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetSphereRadius(500.f);
	DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	DeviceLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("DeviceLight"));
	DeviceLight->SetupAttachment(RootComponent);
	DeviceLight->SetLightColor(FLinearColor::Blue);
	DeviceLight->Intensity = 5000.f;
}

void ACCD_EBlueStick::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority()) // 서버에서 오버랩 판정
	{
		DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACCD_EBlueStick::OnDetectionBeginOverlap);
		DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ACCD_EBlueStick::OnDetectionEndOverlap);
	}
}

void ACCD_EBlueStick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACCD_EBlueStick::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner()) return;

	if (UProgressComponent* ProgressComp = OtherActor->FindComponentByClass<UProgressComponent>())
	{
		if (ProgressComp->ProgressValue > 0.f)
		{
			if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(OtherActor->GetRootComponent()))
			{
				// CustomDepth를 켜고, 스텐실 값을 2로 설정 (멀티캐스트 없이도 복제 설정에 따라 동기화 가능)
				RootPrim->SetRenderCustomDepth(true);
				RootPrim->SetCustomDepthStencilValue(DetectionStencilValue);
				
				UE_LOG(LogTemp, Warning, TEXT("Actor %s entered detection area. Progress: %f"), *OtherActor->GetName(), ProgressComp->ProgressValue);
			}
		}
	}
}

void ACCD_EBlueStick::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	// 감지 영역을 벗어나면 스텐실 값을 초기화 (상호작용 하이라이트와 겹칠 경우를 대비해 0으로 리셋)
	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(OtherActor->GetRootComponent()))
	{
		RootPrim->SetCustomDepthStencilValue(0);
		RootPrim->SetRenderCustomDepth(false);
	}
}

