
#include "Actor/CCD_EBlueStick.h"

#include "Component/ProgressComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Net/UnrealNetwork.h"


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

void ACCD_EBlueStick::BeginPlay()
{
	Super::BeginPlay();
	
	// 머티리얼 인스턴스 생성 (블루프린트에서 StaticMesh에 할당된 재질 인덱스 0번 가정)
	if (MeshComp)
	{
		MeshLightMID = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
		UpdateMeshLightMID();
	}
}

void ACCD_EBlueStick::OnRep_IsLEDOn()
{
}

void ACCD_EBlueStick::UpdateUVLightEffect()
{
	// 1. 장비 자체 라이트 제어
	if (DeviceLight)
	{
		DeviceLight->SetVisibility(bIsLEDOn);
	}

	// 2. MPC를 통한 전역 데칼 효과 제어
	if (UVLightMPC)
	{
		float Intensity = bIsLEDOn ? 1.0f : 0.0f;
		UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), UVLightMPC, FName("UV_Global_Intensity"), Intensity);
	}
	
	// 3. 메쉬 머티리얼 인스턴스 업데이트 (자체 발광 효과)
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
	if (!HasAuthority()) return;
	
	Super::Tick(DeltaTime);
	
	// 자신의 위치와 감지 반경을 MPC에 업데이트 (전역적으로 데칼들이 참조함)
	if (UVLightMPC && bIsLEDOn)
	{
		UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), UVLightMPC, FName("LampPosition"), FLinearColor(GetActorLocation()));
		UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), UVLightMPC, FName("LampRadius"), DetectionSphere->GetUnscaledSphereRadius());
	}
}
