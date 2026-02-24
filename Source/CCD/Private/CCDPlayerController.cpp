
#include "CCDPlayerController.h"

#include "CCDCharacter.h"
#include "CCDPlayerCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ACCDPlayerController::ACCDPlayerController()
{
	PlayerCameraManagerClass = ACCDPlayerCameraManager::StaticClass();
}

void ACCDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 컨트롤러일 때만 UI 생성
	if (IsLocalController() && HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
		}
	}
}

void ACCDPlayerController::StartDeathSpectating()
{
	// 1. 월드 내의 모든 캐릭터 찾기
	TArray<AActor*> AllCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACCDCharacter::StaticClass(), AllCharacters);

	AActor* BestTarget = nullptr;

	for (AActor* Char : AllCharacters)
	{
		// 자기 자신(사망한 본인)이 아니고, 유효한 캐릭터라면 타겟으로 선정
		if (Char != GetPawn() && Char != nullptr)
		{
			BestTarget = Char;
			break; // 우선 첫 번째로 발견된 캐릭터를 선택
		}
	}

	// 2. 카메라 시점 전환
	if (BestTarget)
	{
		// 2초 동안 부드럽게 타겟 캐릭터의 카메라로 이동
		this->SetViewTargetWithBlend(BestTarget, 2.0f, EViewTargetBlendFunction::VTBlend_Cubic);
	}
}