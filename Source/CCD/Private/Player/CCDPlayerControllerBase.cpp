// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CCDPlayerControllerBase.h"

#include "EnhancedInputSubsystems.h"
#include "GameData/CCDGameInstance.h"
#include "GameData/CCD_LoadingSubsystem.h"

void ACCDPlayerControllerBase::Client_StartLoading_Implementation()
{
	if (auto* Subsystem = GetGameInstance()->GetSubsystem<UCCD_LoadingSubsystem>())
	{
		Subsystem->HideLoadingScreen();
		Subsystem->ShowLoadingScreen(LoadingWidgetClass);
	}
}

void ACCDPlayerControllerBase::Client_MoveToEndingLocal_Implementation(const FString& EndingMapPath)
{
	if (UCCDGameInstance* GI = Cast<UCCDGameInstance>(GetGameInstance()))
	{
		GI->TransitionLevel(EndingMapPath);
	}
}

void ACCDPlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	
	// Input
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void ACCDPlayerControllerBase::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	// 새 레벨의 캐릭터가 스폰되면 서브시스템에 UI 제거 요청
	if (auto* Subsystem = GetGameInstance()->GetSubsystem<UCCD_LoadingSubsystem>())
	{
		Subsystem->HideLoadingScreen();
	}
}
