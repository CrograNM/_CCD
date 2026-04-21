#include "GameData/CCDGameMode.h"
#include "Player/CCDCharacter.h"
#include "Player/CCDPlayerController.h"
#include "Actor/ProgressManager.h"
#include "GameData/CCDGameState.h"
#include "Kismet/GameplayStatics.h"

ACCDGameMode::ACCDGameMode()
{
	DefaultPawnClass = ACCDCharacter::StaticClass();
	PlayerControllerClass = ACCDPlayerController::StaticClass();
	
	// 심리스 트래블을 활성화하여 네트워크 연결을 유지한 채 맵을 이동합니다.
	bUseSeamlessTravel = true;
}

void ACCDGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 월드에서 ProgressManager를 찾아 보관합니다.
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AProgressManager::StaticClass());
	ProgressManager = Cast<AProgressManager>(FoundActor);
}

void ACCDGameMode::OnCleaningFinished()
{
	if (ACCDGameState* GS = GetGameState<ACCDGameState>())
	{
		GS->bIsCleaningFinished = true;
		GS->OnRep_CleaningFinished(); 
	}
}

void ACCDGameMode::RequestRespawn(ACCDCharacter* DeadCharacter)
{
	ACCDGameState* GS = GetGameState<ACCDGameState>();
	if (GS && GS->SharedLives > 0)
	{
		// 목숨 차감
		GS->SharedLives--;
		GS->OnRep_SharedLives(); // 서버에서도 UI 갱신을 위해 호출
		UE_LOG(LogTemp, Error, TEXT("Respawning player. Remaining Lives: %d"), GS->SharedLives);

		// 실제 부활 로직 호출 (이전의 N초 타이머 후 호출되도록 연동)
		DeadCharacter->Revive();
	}
	else
	{
		// 목숨이 없으면 게임 오버 처리
		UE_LOG(LogTemp, Error, TEXT("No more lives! Game Over."));
	}
}
