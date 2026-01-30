
#include "CCDPlayerController.h"
#include "CCDPlayerCameraManager.h"
#include "Blueprint/UserWidget.h"

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
