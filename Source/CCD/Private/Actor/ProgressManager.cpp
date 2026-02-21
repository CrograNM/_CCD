
#include "Actor/ProgressManager.h"
#include "Widget/ProgressWidget.h"
#include "Blueprint/UserWidget.h"
#include "CCDGameMode.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


AProgressManager::AProgressManager()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
}

void AProgressManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void AProgressManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProgressManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AProgressManager, CurrentProgress);
	DOREPLIFETIME(AProgressManager, MaxProgress);
}

void AProgressManager::OnRep_CurrentProgress()
{
	UpdateUI();
	
	if (ProgressSound && CurrentProgress > LastProgress) // 진행도가 증가할 때만 사운드 재생
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ProgressSound);
	}
	
	LastProgress = CurrentProgress;
}

void AProgressManager::OnRep_MaxProgress()
{
	UpdateUI();
}

void AProgressManager::AddMaxProgress(float Value)
{
	if (!HasAuthority()) return; // 서버에서만 수정
	MaxProgress += Value; 
	OnRep_MaxProgress();
}

void AProgressManager::AddCurrentProgress(float Value)
{
	if (!HasAuthority()) return; // 서버에서만 수정
	CurrentProgress += Value; 
	OnRep_CurrentProgress();
}

void AProgressManager::UpdateUI()
{
	
	// 로컬 플레이어 컨트롤러 가져오기
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC || !PC->IsLocalController()) return;

	// UI 갱신 로직
	// 일단 간단하게 월드에 생성된 모든 위젯 중 해당 클래스를 찾아 업데이트 -> 나중에 매니저 패턴으로 변경 가능
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UProgressWidget::StaticClass(), false);

	for (UUserWidget* Widget : FoundWidgets)
	{
		if (UProgressWidget* ProgressWidget = Cast<UProgressWidget>(Widget))
		{
			ProgressWidget->UpdatePercent(GetProgressRatio());
			UE_LOG(LogTemp, Warning, TEXT("[UI Updated] Cur: %f, Max: %f, Ratio: %f"), CurrentProgress, MaxProgress, GetProgressRatio());
		}
	}
}

void AProgressManager::UpdateProgress()
{
	// 목표치 달성 확인
	if (CurrentProgress >= MaxProgress)
	{
		ACCDGameMode* GM = Cast<ACCDGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if (GM)
		{
			GM->OnCleaningFinished();
		}
	}
}
