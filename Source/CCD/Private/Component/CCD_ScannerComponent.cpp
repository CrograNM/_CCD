
#include "Component/CCD_ScannerComponent.h"

#include "CCDCharacter.h"
#include "Component/ProgressComponent.h"
#include "Components/WidgetComponent.h"
#include "Widget/ScannerWidget.h"

UCCD_ScannerComponent::UCCD_ScannerComponent()
{
	// 1. 자신의 메쉬 생성 및 부착
	ScannerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScannerMesh"));
	ScannerMesh->SetupAttachment(this);
	ScannerMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	// 2. 3D 위젯 생성 및 메쉬에 부착
	ScannerWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("ScannerWidgetComp"));
	ScannerWidgetComp->SetupAttachment(ScannerMesh, TEXT("ScreenSocket"));
	ScannerWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	ScannerWidgetComp->SetDrawSize(FVector2D(800.f, 600.f));
}

void UCCD_ScannerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCCD_ScannerComponent::UpdateScanner()
{
	float Distance = GetScanActorDistance();
	
	if (!ScannerWidget && ScannerWidgetComp)
	{
		ScannerWidget = Cast<UScannerWidget>(ScannerWidgetComp->GetUserWidgetObject());
	}

	if (ScannerWidget)
	{
		ScannerWidgetComp->SetHiddenInGame(false);
		ScannerWidget->UpdateDistanceDisplay(Distance);
	}
}

float UCCD_ScannerComponent::GetScanActorDistance() const
{
	float ClosestDistance = MaxScanDistance;
	FVector CharacterLocation = GetOwner()->GetActorLocation();
	bool bFound = false;

	for (TObjectIterator<UProgressComponent> It; It; ++It)
	{
		if (It->GetWorld() != GetWorld()) continue;
		AActor* TargetActor = It->GetOwner();
		if (!TargetActor || TargetActor == GetOwner() || It->ProgressValue <= 0.f) continue;

		float Distance = FVector::Dist(CharacterLocation, TargetActor->GetActorLocation());
		
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance; 
			bFound = true;
		}
	}
	return bFound ? ClosestDistance : -1.f;
}