#include "Actor/Decal_StainActor_Base.h"
#include "Component/ProgressComponent.h"
#include "Component/WashableComponent.h"
#include "Components/DecalComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"

ADecal_StainActor_Base::ADecal_StainActor_Base()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bNetLoadOnClient = true;
	SetReplicatingMovement(true);
	
	ProgressComp = CreateDefaultSubobject<UProgressComponent>(TEXT("ProgressComp"));
	ProgressComp->SetIsReplicated(true);
	WashableComp = CreateDefaultSubobject<UWashableComponent>(TEXT("WashableComp"));
	WashableComp->SetIsReplicated(true);
	
	GetDecal()->SetIsReplicated(true);
}

void ADecal_StainActor_Base::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 에디터에서 WashableType에 따라 Decal Material 변경
	if (WashableComp)
	{
		switch (WashableComp->GetWashableType())
		{
		case ECCD_WashableType::EWT_Blood:
			GetDecal()->SetDecalMaterial(BloodDecalMaterial);
			SetPollution(1.f, 0.f);
			break;
		case ECCD_WashableType::EWT_Excrement:
			GetDecal()->SetDecalMaterial(ExcrementDecalMaterial);
			SetPollution(0.f, 1.f);
			break;
		case ECCD_WashableType::EWT_Water:
			GetDecal()->SetDecalMaterial(WaterDecalMaterial);
			SetPollution(0.f, 0.f);
			break;
		}
	}
}

void ADecal_StainActor_Base::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetDecal())
	{
		DecalDMI = GetDecal()->CreateDynamicMaterialInstance();
	}
	UpdateDecalMaterial();
}

void ADecal_StainActor_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 세척이 완료되어 수명이 설정된 경우, 점점 투명해지도록 처리
	if (GetLifeSpan() > 0.f && DecalDMI)
	{
		const float Opacity = GetLifeSpan() / InitialLifeSpan;
		DecalDMI->SetScalarParameterValue(TEXT("Opacity"), Opacity);
	}
}

void ADecal_StainActor_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADecal_StainActor_Base, Pollution_Blood);
	DOREPLIFETIME(ADecal_StainActor_Base, Pollution_Excrement);
}

void ADecal_StainActor_Base::UpdateDecalMaterial() const
{
	if (DecalDMI)
	{
		DecalDMI->SetScalarParameterValue(TEXT("WashHealthRatio"), WashableComp->GetWashHealthRatio());
		DecalDMI->SetScalarParameterValue(TEXT("BloodIntensity"), Pollution_Blood);
		DecalDMI->SetScalarParameterValue(TEXT("ExcrementIntensity"), Pollution_Excrement);
	}
}

void ADecal_StainActor_Base::SetPollution(float InBlood, float InExcrement)
{
	if (HasAuthority())
	{
		Pollution_Blood = InBlood;
		Pollution_Excrement = InExcrement;
		UpdateDecalMaterial();
	}
}
