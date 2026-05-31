// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/LevelExit.h"

#include "EngineUtils.h"
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
	
	int32 TotalOverlappingLivingPlayersCount = 0;
	
	// 여러 출구의 영역이 겹쳐있을 때 동일한 플레이어가 중복 카운트되는 것을 방지하기 위한 Set
	TSet<ACCDCharacter*> CountedPlayers;
	
	for (TActorIterator<ALevelExit> It(GetWorld()); It; ++It)
	{
		ALevelExit* ExitActor = *It;
		// 출구 액터와 해당 출구의 WaitingArea 컴포넌트가 유효한지 확인
		if (ExitActor && ExitActor->WaitingArea)
		{
			TArray<AActor*> OverlappingActors;
			ExitActor->WaitingArea->GetOverlappingActors(OverlappingActors);

			for (AActor* Actor : OverlappingActors)
			{
				if (ACCDCharacter* Character = Cast<ACCDCharacter>(Actor))
				{
					// 영역 안에 있고, 죽지 않았으며, 아직 카운트하지 않은 플레이어라면
					if (!Character->IsDead() && !CountedPlayers.Contains(Character))
					{
						CountedPlayers.Add(Character);
						TotalOverlappingLivingPlayersCount++;
					}
				}
			}
		}
	}

	// 살아있는 플레이어가 1명이라도 있고, 모든 출구에 분산되어 있더라도 그들의 합산이 전체 생존자 수와 일치하면 true
	return (LivingPlayersCount > 0 && TotalOverlappingLivingPlayersCount >= LivingPlayersCount);
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