
#include "GameData/CCDGameState.h"

#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"

void ACCDGameState::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		bIsCleaningFinished = false; 
		bIsGameOver = false;
	}
}

void ACCDGameState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCDGameState, bIsCleaningFinished);
	DOREPLIFETIME(ACCDGameState, ReplicatedClearedMapPaths);
	DOREPLIFETIME(ACCDGameState, bIsGameOver);
}

void ACCDGameState::OnRep_CleaningFinished()
{
}

void ACCDGameState::OnRep_IsGameOver()
{
	if (bIsGameOver && GameOverWidgetClass)
	{
		// 각 클라이언트(서버 포함)의 로컬 화면에 UI 생성 및 출력
		if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
		{
			if (LocalPC->IsLocalController())
			{
				UUserWidget* GameOverWidget = CreateWidget<UUserWidget>(LocalPC, GameOverWidgetClass);
				if (GameOverWidget)
				{
					GameOverWidget->AddToViewport(999); // 최상단 노출
                    
					// 필요 시 마우스 커서 활성화 및 입력 모드 변경
					LocalPC->SetShowMouseCursor(true);
					FInputModeUIOnly InputMode;
					InputMode.SetWidgetToFocus(GameOverWidget->TakeWidget());
					LocalPC->SetInputMode(InputMode);
				}
			}
		}
	}
}


