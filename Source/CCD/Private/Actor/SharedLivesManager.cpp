// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/SharedLivesManager.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/LivesWidget.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASharedLivesManager::ASharedLivesManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ASharedLivesManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASharedLivesManager, Lives);
}

void ASharedLivesManager::Server_SetLives_Implementation(int32 NewLives)
{
	if (!HasAuthority()) return;
	Lives = NewLives;
	OnRep_Lives();
}

// Called when the game starts or when spawned
void ASharedLivesManager::BeginPlay()
{
	Super::BeginPlay();
}

bool ASharedLivesManager::AttemptDecrementLife()
{
	if (!HasAuthority()) return false;

	if (bIsInfiniteLives) return true;

	if (Lives > 0)
	{
		Lives--;
		OnRep_Lives(); // 서버에서 즉시 UI 업데이트
		return true;
	}

	return false;
}

void ASharedLivesManager::OnRep_Lives()
{
	UE_LOG(LogTemp, Warning, TEXT("Lives Changed: %d"), Lives);
	UpdateLivesUI();
}


void ASharedLivesManager::UpdateLivesUI()
{
	// 로컬 플레이어 컨트롤러 가져오기
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC || !PC->IsLocalController()) return;

	// UI 갱신 로직
	// 일단 간단하게 월드에 생성된 모든 위젯 중 해당 클래스를 찾아 업데이트 -> 나중에 매니저 패턴으로 변경 가능
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, ULivesWidget::StaticClass(), false);

	for (UUserWidget* Widget : FoundWidgets)
	{
		if (ULivesWidget* LivesWidget = Cast<ULivesWidget>(Widget))
		{
			LivesWidget->UpdateLivesDisplay(Lives);
		}
	}
}