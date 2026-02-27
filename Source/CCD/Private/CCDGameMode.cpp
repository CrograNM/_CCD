#include "CCDGameMode.h"
#include "CCDCharacter.h"
#include "CCDPlayerController.h"
#include "Actor/ProgressManager.h"
#include "Kismet/GameplayStatics.h"

ACCDGameMode::ACCDGameMode()
{
	DefaultPawnClass = ACCDCharacter::StaticClass();
	PlayerControllerClass = ACCDPlayerController::StaticClass();
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
	// 여기서 게임 종료 처리 (예: 결과창 UI 띄우기 등)
	UE_LOG(LogTemp, Warning, TEXT("모든 청소가 완료되었습니다! 스테이지 클리어!"));
}

void ACCDGameMode::RestartPlayer(AController* NewPlayer)
{
	if (NewPlayer == nullptr) return;

	// 1. 기존에 조종하던 캐릭터가 있다면 파괴 처리 (부활 시 교체 방식)
	if (APawn* OldPawn = NewPlayer->GetPawn())
	{
		OldPawn->Destroy();
	}

	// 부모 클래스의 RestartPlayer를 호출 -> 월드의 'PlayerStart' 위치를 자동으로 찾아 스폰
	Super::RestartPlayer(NewPlayer);
    
	UE_LOG(LogTemp, Warning, TEXT("플레이어 부활함"));
}