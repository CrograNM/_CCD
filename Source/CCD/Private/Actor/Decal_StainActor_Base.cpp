
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
	
	// 부모 클래스인 ADecalActor가 가진 Decal 컴포넌트를 가져옴
	UDecalComponent* DecalComp = GetDecal();
	if (DecalComp)
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
}

void ADecal_StainActor_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADecal_StainActor_Base, Rep_StainColor);
}

UMaterialInstanceDynamic* ADecal_StainActor_Base::GetDecalDMI()
{
	if (DecalDMI) return DecalDMI;

	UDecalComponent* DecalComp = GetDecal();
	if (DecalComp)
	{
		DecalDMI = DecalComp->CreateDynamicMaterialInstance();
	}
	return DecalDMI;
}

void ADecal_StainActor_Base::OnRep_StainColor()
{
	if (UMaterialInstanceDynamic* DMI = GetDecalDMI())
	{
		DMI->SetVectorParameterValue(TEXT("BaseColor Tint"), Rep_StainColor);
	}
}

void ADecal_StainActor_Base::SetStainColor(FLinearColor NewColor)
{
	if (HasAuthority())
	{
		Rep_StainColor = NewColor;
		OnRep_StainColor(); // 서버에서도 즉시 적용
	}
}

void ADecal_StainActor_Base::UpdateDecalOpacity(float NewRatio) const
{
	if (DecalDMI)
	{
		// 머티리얼의 Scalar Parameter 업데이트
		DecalDMI->SetScalarParameterValue(TEXT("Opacity Intensity"), NewRatio);
	}
}
