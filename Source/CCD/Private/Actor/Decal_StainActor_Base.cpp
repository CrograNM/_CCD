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
		UMaterialInterface* NewMaterial = nullptr;
		switch (WashableComp->GetWashableType())
		{
		case ECCD_WashableType::EWT_Blood:
			NewMaterial = BloodDecalMaterial;
			if (!GetWorld()->IsGameWorld()) SetPollution(1.f, 0.f);
			break;
		case ECCD_WashableType::EWT_Excrement:
			NewMaterial = ExcrementDecalMaterial;
			if (!GetWorld()->IsGameWorld()) SetPollution(0.f, 1.f);
			break;
		case ECCD_WashableType::EWT_Water:
			NewMaterial = WaterDecalMaterial;
			if (!GetWorld()->IsGameWorld()) SetPollution(0.f, 0.f);
			break;
		}
		GetDecal()->SetDecalMaterial(NewMaterial);
		
		// 게임 플레이 도중(스폰 이후) 재질이 변경되었다면, DMI를 현재 재질에 맞게 재생성해야 합니다.
		// SpillWater 함수가 스폰 직후 타입을 변경하므로, BeginPlay에서 만든 DMI는 무효화됩니다.
		if (HasActorBegunPlay() && GetDecal())
		{
			DecalDMI = GetDecal()->CreateDynamicMaterialInstance();
			UpdateDecalMaterial(); // 새로운 DMI에 파라미터(오염도 등) 다시 적용
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
	
	// 클라이언트에서 스폰 시점에 이미 씻겨진 상태(bIsCleaned = true)로 넘어온 경우
	if (bIsCleaned)
	{
		OnRep_IsCleaned();
	}
}

void ADecal_StainActor_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 세척이 완료되어 수명이 설정된 경우, 점점 투명해지도록 처리
	if (bIsCleaned && DecalDMI)
	{
		// 시간을 누적하여 직접 투명도 계산 (LifeSpan 의존성 제거)
		FadeTimeAccumulator += DeltaTime;
        
		// 5.0초 동안 서서히 투명해짐 (SetLifeSpan 값과 일치시킴)
		const float FadeDuration = 5.0f;
		const float Opacity = FMath::Clamp(1.0f - (FadeTimeAccumulator / FadeDuration), 0.0f, 1.0f);
        
		DecalDMI->SetScalarParameterValue(TEXT("Opacity"), Opacity);
	}
}

void ADecal_StainActor_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADecal_StainActor_Base, Pollution_Blood);
	DOREPLIFETIME(ADecal_StainActor_Base, Pollution_Excrement);
	DOREPLIFETIME(ADecal_StainActor_Base, bIsCleaned);
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

void ADecal_StainActor_Base::NotifyCleaned()
{
	if (HasAuthority())
	{
		bIsCleaned = true;
		OnRep_IsCleaned(); // 서버에서도 즉시 실행
	}
}

void ADecal_StainActor_Base::OnRep_IsCleaned()
{
	SetLifeSpan(5.0f);
}