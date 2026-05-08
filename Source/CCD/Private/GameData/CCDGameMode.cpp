#include "GameData/CCDGameMode.h"
#include "Player/CCDCharacter.h"
#include "Player/CCDPlayerController.h"
#include "Actor/ProgressManager.h"
#include "Actor/SharedLivesManager.h"
#include "GameData/CCDGameRecordSubsystem.h"
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
	
	// 맵이 클리어 됐다는 사실을 기록
	if (UCCDGameRecordSubsystem* RecordSystem = GetGameInstance()->GetSubsystem<UCCDGameRecordSubsystem>())
	{
		FString CurrentMapName = GetWorld()->GetOutermost()->GetName();
		RecordSystem->RecordMapClear(CurrentMapName);
	}
}

void ACCDGameMode::RequestRespawn(ACCDCharacter* DeadCharacter)
{
	// 월드에서 매니저를 찾아 부활 가능 여부를 확인합니다.
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASharedLivesManager::StaticClass());
	ASharedLivesManager* LivesManager = Cast<ASharedLivesManager>(FoundActor);

	if (LivesManager && LivesManager->AttemptDecrementLife())
	{
		DeadCharacter->Revive();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No more lives or Manager not found!"));
	}
}

void ACCDGameMode::TransitionToLevel(const FString& NextLevelPath)
{
	if (NextLevelPath.IsEmpty()) return;

	// 1. 모든 접속된 플레이어 컨트롤러를 순회하며 로딩 UI 출력을 명령합니다.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ACCDPlayerControllerBase* PC = Cast<ACCDPlayerControllerBase>(It->Get()))
		{
			PC->Client_StartLoading(); // RPC 호출
		}
	}

	// 2. 클라이언트가 UI를 띄우고 네트워크 패킷을 처리할 시간을 준 뒤 이동합니다.
	FTimerHandle TravelTimer;
	GetWorldTimerManager().SetTimer(TravelTimer, [this, NextLevelPath]()
	{
		// 심리스 트래블 활성 상태이므로 연결을 유지하며 이동합니다.
		GetWorld()->ServerTravel(NextLevelPath + TEXT("?listen"));
	}, 1.0f, false);
}
