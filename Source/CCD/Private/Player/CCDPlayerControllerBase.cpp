// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CCDPlayerControllerBase.h"

#include "GameData/CCD_LoadingSubsystem.h"

void ACCDPlayerControllerBase::Client_StartLoading_Implementation()
{
	if (auto* Subsystem = GetGameInstance()->GetSubsystem<UCCD_LoadingSubsystem>())
	{
		Subsystem->HideLoadingScreen();
		Subsystem->ShowLoadingScreen(LoadingWidgetClass);
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