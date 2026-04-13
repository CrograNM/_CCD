


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

void ACCD_EScannerActor::OnEquipped()
{
	// 장착 시 자동으로 스캔 시작
	if (HasAuthority())
	{
		bIsScanning = true;
		OnRep_IsScanning(); // 서버에서도 타이머 작동을 위해 호출
	}
}

void ACCD_EScannerActor::OnUnequipped()
{
	// 해제 시 스캔 중지 및 UI 숨김
	if (HasAuthority())
	{
		bIsScanning = false;
		OnRep_IsScanning();
	}
    
	// 클라이언트 UI 즉시 초기화
	if (ScannerWidgetComp)
	{
		ScannerWidgetComp->SetHiddenInGame(true);
	}
	ScannerDistance = -1.0f;
}

void ACCD_EScannerActor::ExecuteAction()
{
	if (!HasAuthority()) return;
	bIsScanning = !bIsScanning;
	if (bIsScanning)
	{
		// 애니메이션 후 스캔 시작 (아직 애니메이션 없음)
		GetWorldTimerManager().SetTimer(
			ScannerTimerHandle, 
			this, 
			&ACCD_EScannerActor::PerformScan, 
			1.0f, 
			false
		);
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
	if (bIsScanning)
	{
		GetWorld()->GetTimerManager().SetTimer(ScannerTimerHandle, this, &ACCD_EScannerActor::PerformScan, 0.5f, true);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(ScannerTimerHandle);
		// UI 업데이트를 위해 거리 초기화 RPC 등을 호출할 수 있음
		UpdateScannerUI(); 
	}
}
void ACCD_EScannerActor::PerformScan()
{
	if (!HasAuthority() || !bIsScanning) return;

	// 거리 계산 후 변수 갱신
	ScannerDistance = GetScanActorDistance();
	
	// 모든 클라이언트의 UI 수치 갱신을 위해 멀티캐스트 호출
	Multicast_UpdateScannerUI();
	
	// 스캔 간격을 거리 기반으로 조절 (거리가 멀수록 간격 증가)
	float CurrentDistance = (ScannerDistance < 0.f) ? MaxScanDistance : ScannerDistance;
	float NextInterval = FMath::GetMappedRangeValueClamped(
		FVector2D(0.f, MaxScanDistance), 
		FVector2D(0.5f, 1.5f), 
		CurrentDistance
	);
	
	GetWorldTimerManager().SetTimer(
		ScannerTimerHandle, 
		this, 
		&ACCD_EScannerActor::PerformScan, 
		NextInterval, 
		false
	);
}

void ACCD_EScannerActor::Multicast_UpdateScannerUI_Implementation()
{
	UpdateScannerUI();
}

void ACCD_EScannerActor::UpdateScannerUI()
{
	if (!bIsScanning)
	{
		if (ScannerWidgetComp)
		{
			ScannerWidgetComp->SetHiddenInGame(true);
		}
		return;
	}
	
	if (ScannerSound && ScannerDistance >= 0.f) // 유효한 거리 값이 있을 때만 사운드 재생
	{
		float CurrentDistance = (ScannerDistance < 0.f) ? MaxScanDistance : ScannerDistance;
		float PitchRate = FMath::GetMappedRangeValueClamped(
				FVector2D(0.f, MaxScanDistance), 
				FVector2D(1.0f, 0.5f), 
				CurrentDistance
			);
		UGameplayStatics::PlaySoundAtLocation(this, ScannerSound, GetActorLocation(), 1.0, PitchRate); 
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
