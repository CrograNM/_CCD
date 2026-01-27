#include "CCDGameMode.h"
#include "CCDCharacter.h"
#include "CCDPlayerController.h"
#include "Actor/ProgressManager.h"
#include "Kismet/GameplayStatics.h"

ACCDGameMode::ACCDGameMode()
{
	// 1. 기본 캐릭터 클래스 등록
	DefaultPawnClass = ACCDCharacter::StaticClass();

	// 2. 기본 컨트롤러 클래스 등록
	PlayerControllerClass = ACCDPlayerController::StaticClass();

	// 3. HUD 클래스가 있다면 여기서 등록 (나중에 만들 HUD)
	// HUDClass = ACCDHUD::StaticClass();
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