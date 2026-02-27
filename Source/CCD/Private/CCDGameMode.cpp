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