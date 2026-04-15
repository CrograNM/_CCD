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