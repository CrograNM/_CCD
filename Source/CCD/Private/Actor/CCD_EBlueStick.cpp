
#include "Actor/CCD_EBlueStick.h"

#include "Component/ProgressComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/KismetMaterialLibrary.h"


ACCD_EBlueStick::ACCD_EBlueStick()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.033f;
	
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
}

void ACCD_EBlueStick::Tick(float DeltaTime)
{
	if (!HasAuthority()) return;
	
	Super::Tick(DeltaTime);
	
	// 자신의 위치와 감지 반경을 MPC에 업데이트 (전역적으로 데칼들이 참조함)
	if (UVLightMPC)
	{
		UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), UVLightMPC, FName("LampPosition"), FLinearColor(GetActorLocation()));
		UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), UVLightMPC, FName("LampRadius"), DetectionSphere->GetUnscaledSphereRadius());
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("[EBlueStick] UVLightMPC is not assigned!"));
	}
}