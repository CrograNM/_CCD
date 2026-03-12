


#include "Actor/CCD_EScannerActor.h"

#include <Net/UnrealNetwork.h>

#include "Component/ProgressComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/ScannerWidget.h"

ACCD_EScannerActor::ACCD_EScannerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	// 메쉬 생성 및 루트 설정
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// 기본적으로 물리 연산, 콜리전 무시
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	
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
	DOREPLIFETIME(ACCD_EScannerActor, bIsScanning);
}

void ACCD_EScannerActor::ExecuteAction()
{
	if (!HasAuthority()) return;
	bIsScanning = !bIsScanning;
	if (bIsScanning)
	{
		GetWorldTimerManager().SetTimer(ScannerTimerHandle, this, &ACCD_EScannerActor::PerformScan, ScanInterval, true, 0.0f);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(ScannerTimerHandle);
		ScannerDistance = -1.0f; // 거리 초기화
		UpdateScannerUI();
	}
	// 서버에서도 즉시 UI 상태 반영
	OnRep_IsScanning();
}

void ACCD_EScannerActor::OnRep_IsScanning()
{
	// if (bIsScanning)
	// {
	// 	if (ScannerWidgetComp) ScannerWidgetComp->SetHiddenInGame(false);
	// }
}
void ACCD_EScannerActor::PerformScan()
{
	if (!HasAuthority()) return;

	// 거리 계산 후 변수 갱신
	ScannerDistance = GetScanActorDistance();
	
	// 모든 클라이언트의 UI 수치 갱신을 위해 멀티캐스트 호출
	Multicast_UpdateScannerUI();
}

void ACCD_EScannerActor::Multicast_UpdateScannerUI_Implementation()
{
	UpdateScannerUI();
}

void ACCD_EScannerActor::UpdateScannerUI()
{
	if (ScannerSound && ScannerDistance >= 0.f) // 유효한 거리 값이 있을 때만 사운드 재생
	{
		float ClampedDistance = FMath::Clamp(ScannerDistance, 0.f, MaxScanDistance);
		float VolumeRate =  1.0 - (ClampedDistance / MaxScanDistance); // 거리에 따라 볼륨 조절
		UGameplayStatics::PlaySoundAtLocation(this, ScannerSound, GetActorLocation(), VolumeRate); 
	}
	
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
