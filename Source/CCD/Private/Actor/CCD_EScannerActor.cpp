


#include "Actor/CCD_EScannerActor.h"

#include <Net/UnrealNetwork.h>

#include "Component/ProgressComponent.h"
#include "Components/WidgetComponent.h"
#include "Widget/ScannerWidget.h"

ACCD_EScannerActor::ACCD_EScannerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	// 2. 3D 위젯 생성 및 자기 자신에게 부착
	ScannerWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("ScannerWidgetComp"));
	ScannerWidgetComp->SetupAttachment(MeshComp, TEXT("ScreenSocket")); 
	ScannerWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	ScannerWidgetComp->SetDrawSize(FVector2D(800.f, 600.f));
}

void ACCD_EScannerActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCD_EScannerActor, ScannerDistance);
}

void ACCD_EScannerActor::ExecuteAction()
{
	// UE_LOG(LogTemp, Warning, TEXT("[Scanner] ExecuteAction"));
	// 서버에서 거리를 계산하여 변수에 담습니다.
	if (HasAuthority())
	{
		ScannerDistance = GetScanActorDistance();
		Multicast_UpdateScannerUI();
	}
}

void ACCD_EScannerActor::Multicast_UpdateScannerUI_Implementation()
{
	UpdateScannerUI();
}

void ACCD_EScannerActor::UpdateScannerUI()
{
	if (!ScannerWidget && ScannerWidgetComp)
	{
		ScannerWidget = Cast<UScannerWidget>(ScannerWidgetComp->GetUserWidgetObject());
	}

	if (ScannerWidget)
	{
		ScannerWidgetComp->SetHiddenInGame(false);
		// 복제된 ScannerDistance 값을 UI에 반영합니다.
		ScannerWidget->UpdateDistanceDisplay(ScannerDistance);
	}
}

float ACCD_EScannerActor::GetScanActorDistance() const
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
