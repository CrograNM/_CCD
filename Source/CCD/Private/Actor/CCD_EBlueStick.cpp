
#include "Actor/CCD_EBlueStick.h"

#include "Actor/CCD_EBlueStickManagerSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Net/UnrealNetwork.h"


ACCD_EBlueStick::ACCD_EBlueStick()
{
	PrimaryActorTick.bCanEverTick = false;
	// PrimaryActorTick.TickInterval = 0.033f;
	
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
	DeviceLight->AttenuationRadius = 500.f;
	
	DeviceLight->SetVisibility(false);
}

void ACCD_EBlueStick::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCD_EBlueStick, bIsLEDOn); // 변수 동기화 등록
}

void ACCD_EBlueStick::ExecuteAction()
{
	if (!HasAuthority()) return;
	bIsLEDOn = !bIsLEDOn; // LED 상태 토글
	UpdateUVLightEffect();
}

void ACCD_EBlueStick::OnEquipped()
{
	if (HasAuthority())
	{
		bIsLEDOn = true; // 장착 시 LED 켜기
		UpdateUVLightEffect();
	}
}

void ACCD_EBlueStick::OnUnequipped()
{
	if (HasAuthority())
	{
		bIsLEDOn = false; // 해제 시 LED 끄기
		UpdateUVLightEffect();
	}
}

void ACCD_EBlueStick::ExecuteBlueStick()
{
	ExecuteAction();
}

void ACCD_EBlueStick::BeginPlay()
{
	Super::BeginPlay();
	
	// 월드 서브시스템에 자신 등록 (MPC 업데이트를 위해)
	if (auto* Subsystem = GetWorld()->GetSubsystem<UCCD_EBlueStickManagerSubsystem>())
	{
		Subsystem->RegisterStick(this);
	}
	
	// 머티리얼 인스턴스 생성 (블루프린트에서 StaticMesh에 할당된 재질 인덱스 0번 가정)
	if (MeshComp)
	{
		MeshLightMID = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
		UpdateMeshLightMID();
	}
	
	UpdateUVLightEffect();
}

void ACCD_EBlueStick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 월드 서브시스템에서 자신 해제
	if (auto* Subsystem = GetWorld()->GetSubsystem<UCCD_EBlueStickManagerSubsystem>())
	{
		Subsystem->UnregisterStick(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void ACCD_EBlueStick::OnRep_IsLEDOn()
{
	UpdateUVLightEffect();
}

void ACCD_EBlueStick::UpdateUVLightEffect()
{
	// 1. 장비 자체 라이트 제어
	if (DeviceLight)
	{
		DeviceLight->SetVisibility(bIsLEDOn);
	}
	
	// 2. 메쉬 머티리얼 인스턴스 업데이트 (자체 발광 효과)
	UpdateMeshLightMID();
}

void ACCD_EBlueStick::UpdateMeshLightMID()
{
	if (MeshLightMID)
	{
		FLinearColor EmissiveColor = bIsLEDOn ? FLinearColor(0.0f, 5.0f, 10.0f, 1.0f) :FLinearColor(0.12f, 0.16f, 0.21f, 1.0f);
		MeshLightMID->SetVectorParameterValue(FName("EmissiveColor"), EmissiveColor);
	}
}

void ACCD_EBlueStick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
