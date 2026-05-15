// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/LevelExit.h"
#include "GameData/CCDGameState.h"
#include "Player/CCDCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerState.h"

ALevelExit::ALevelExit()
{
	bNeedToCheckProgressOver = true;
}

bool ALevelExit::IsWaitingAreaFull() const
{
	if (!WaitingArea) return false;

	ACCDGameState* GS = GetWorld()->GetGameState<ACCDGameState>();
	if (!GS) return false;

	// 청소 완료 여부 우선 확인
	if (!GS->bIsCleaningFinished) return false;

	int32 LivingPlayersCount = 0;
	int32 OverlappingLivingPlayersCount = 0;

	// 서버 내 모든 플레이어 중 살아있는 인원만 카운트
	for (APlayerState* PS : GS->PlayerArray)
	{
		if (PS)
		{
			if (ACCDCharacter* Character = Cast<ACCDCharacter>(PS->GetPawn()))
			{
				if (!Character->IsDead()) 
				{
					LivingPlayersCount++;
				}
			}
		}
	}

	// WaitingArea 안에 있는 액터 중 살아있는 플레이어 카운트
	TArray<AActor*> OverlappingActors;
	WaitingArea->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (ACCDCharacter* Character = Cast<ACCDCharacter>(Actor))
		{
			// 영역 안에 있어도 죽은 상태라면 카운트에서 제외
			if (!Character->IsDead())
			{
				OverlappingLivingPlayersCount++;
			}
		}
	}

	// 살아있는 플레이어가 1명이라도 있고, 그들이 모두 영역에 들어왔을 때 true
	return (LivingPlayersCount > 0 && OverlappingLivingPlayersCount >= LivingPlayersCount);
}

void ALevelExit::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;

	ACCDGameState* GS = GetWorld()->GetGameState<ACCDGameState>();
	if (!GS) return;

	// 청소 미완료 시 로그 출력
	if (!GS->bIsCleaningFinished)
	{
		UE_LOG(LogTemp, Warning, TEXT("탈출 실패: 아직 구역이 충분히 깨끗하지 않습니다."));
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Cleaning progress is insufficient!"));
		}
		return;
	}

	// 인원 부족 시 로그 출력
	if (!IsWaitingAreaFull())
	{
		UE_LOG(LogTemp, Warning, TEXT("탈출 실패: 모든 생존자가 집결 구역에 모여야 합니다."));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("All surviving players must gather in the waiting area!"));
		}
		return;
	}
	
	Super::Interact_Implementation(Interactor);
}