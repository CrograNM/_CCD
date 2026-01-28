// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ProgressManager.h"
#include "Widget/ProgressWidget.h"
#include "Blueprint/UserWidget.h"
#include "CCDGameMode.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AProgressManager::AProgressManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AProgressManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
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

void AProgressManager::OnRep_Progress()
{
	UpdateUI();
}

void AProgressManager::AddMaxProgress(float Value)
{
	if (!HasAuthority()) return; // 서버에서만 수정
	MaxProgress += Value; 
	UpdateUI(); // 서버 화면 갱신
}

void AProgressManager::AddCurrentProgress(float Value)
{
	if (!HasAuthority()) return; // 서버에서만 수정
	CurrentProgress += Value; 
	UpdateUI(); // 서버 화면 갱신
}

void AProgressManager::UpdateUI()
{
	// 데디케이티드 서버에서는 UI 갱신하지 않음 -> 현재 작품은 리슨 서버 이므로 생략
	
	// 로컬 플레이어 컨트롤러 가져오기
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
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
