
#include "Actor/Decal_StainActor_Base.h"

#include "Component/ProgressComponent.h"
#include "Component/WashableComponent.h"
#include "Components/DecalComponent.h"
#include "Net/UnrealNetwork.h"

ADecal_StainActor_Base::ADecal_StainActor_Base()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 네트워크 복제 설정
	bReplicates = true;
	bNetLoadOnClient = true;
	SetReplicatingMovement(true);
	
	// 컴포넌트 생성 및 포함
	ProgressComp = CreateDefaultSubobject<UProgressComponent>(TEXT("ProgressComp"));
	ProgressComp->SetIsReplicated(true);
	WashableComp = CreateDefaultSubobject<UWashableComponent>(TEXT("WashableComp"));
	WashableComp->SetIsReplicated(true);
	
	// 부모 데칼 컴포넌트의 복제 설정
	GetDecal()->SetIsReplicated(true);
}

void ADecal_StainActor_Base::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorTickEnabled(false); // 초기에는 Tick 비활성화 -> 양동이에서 스폰 시 조건에 맞춰 활성화
	
	// 부모 클래스인 ADecalActor가 가진 Decal 컴포넌트를 가져옴
	UDecalComponent* DecalComp = GetDecal();
	if (DecalComp && WashableComp->GetWashableType() != ECCD_WashableType::EWT_Water)
	{
		// 0번 슬롯의 머티리얼로 DMI 생성
		UMaterialInterface* BaseMat = DecalComp->GetDecalMaterial();
		if (BaseMat)
		{
			DecalDMI = DecalComp->CreateDynamicMaterialInstance();
		}
	}
	UpdateDecalOpacity(WashableComp->GetWashHealthRatio());
}

void ADecal_StainActor_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UE_LOG(LogTemp, Warning, TEXT("%s - Tick : %f"), *GetName(), DeltaTime); // Tick 최적화 검증용
	
	if (WashableComp->GetWashableType() == ECCD_WashableType::EWT_Water && ProgressComp->ProgressValue <= 0.0f)
	{
		// 시간을 누적하여 직접 투명도 계산 (LifeSpan 의존성 제거)
		FadeTimeAccumulator += DeltaTime;
        
		// 서서히 투명해짐
		const float FadeDuration = 3.0f;
		const float Opacity = FMath::Clamp(1.0f - (FadeTimeAccumulator / FadeDuration), 0.0f, 1.0f);
        
		UpdateDecalOpacity(Opacity);
		
		if (Opacity <= 0.0f)
		{
			Destroy();
		}
	}
}

void ADecal_StainActor_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);	
	DOREPLIFETIME(ADecal_StainActor_Base, DecalColor);
}

void ADecal_StainActor_Base::UpdateDecalOpacity(float NewRatio) const
{
	if (DecalDMI)
	{
		// 머티리얼의 Scalar Parameter 업데이트
		DecalDMI->SetScalarParameterValue(TEXT("Opacity Intensity"), NewRatio);
	}
}

void ADecal_StainActor_Base::UseWaterDecalMaterial()
{
	UDecalComponent* DecalComp = GetDecal();
	DecalComp->SetDecalMaterial(WaterDecalMaterial);
	
	UMaterialInterface* BaseMat = DecalComp->GetDecalMaterial();
	if (BaseMat)
	{
		DecalDMI = DecalComp->CreateDynamicMaterialInstance();
		
		if (DecalDMI)
		{
			DecalDMI->SetVectorParameterValue(TEXT("BaseColor Tint"), DecalColor);
		}
	}
	
	UpdateDecalOpacity(WashableComp->GetWashHealthRatio());
}

void ADecal_StainActor_Base::OnRep_DecalColor()
{
	if (DecalDMI)
	{
		DecalDMI->SetVectorParameterValue(TEXT("BaseColor Tint"), DecalColor);
	}
}
