
#include "CCDPlayerController.h"

#include "CCDCharacter.h"
#include "CCDPlayerCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Component/CCD_DeathComponent.h"
#include "Component/CCD_ViewComponent.h"
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

void ACCDPlayerController::StartDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("[PlayerController] StartDeath"));
	HandleTargetDeath(nullptr);
}

void ACCDPlayerController::UpdateDeathVisuals()
{
	PostProcessAlpha = FMath::Clamp(PostProcessAlpha + 0.01f, 0.0f, 1.0f);
	
	// 카메라의 포스트 프로세스 설정을 실시간으로 업데이트
	if (ACCDCharacter* MyChar = Cast<ACCDCharacter>(GetPawn())) 
	{
		FPostProcessSettings& PPS = MyChar->GetFirstPersonCamera()->PostProcessSettings;
		PPS.bOverride_ColorSaturation = true;
		PPS.ColorSaturation = FVector4(1.0f - PostProcessAlpha, 1.0f - PostProcessAlpha, 1.0f - PostProcessAlpha, 1.0f);
		
		FPostProcessSettings& PPS3rd = MyChar->GetFollowCamera()->PostProcessSettings;
		PPS3rd.bOverride_ColorSaturation = true;
		PPS3rd.ColorSaturation = FVector4(1.0f - PostProcessAlpha, 1.0f - PostProcessAlpha, 1.0f - PostProcessAlpha, 1.0f);
	}

	if (PostProcessAlpha >= 1.0f) GetWorldTimerManager().ClearTimer(PostProcessTimer);
}

void ACCDPlayerController::SpectateNextPlayer()
{
	// 월드의 살아있는 캐릭터들을 순회하며 다음 타겟 선정
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACCDCharacter::StaticClass(), Players);

	for (AActor* P : Players)
	{
		if (P == GetPawn()) continue; // 본인 제외
		
		UCCD_DeathComponent* DeathComp = P->FindComponentByClass<UCCD_DeathComponent>();
		if (DeathComp && !DeathComp->IsDead())
		{
			CurrentSpectateTarget = P;
			// 시점 전환 블렌딩
			SetViewTargetWithBlend(P, 0.5f); 
			
			// 대상이 죽었을 때 다시 타겟을 바꾸기 위해 이벤트 구독
			DeathComp->OnDeath.AddDynamic(this, &ACCDPlayerController::HandleTargetDeath);
			break;
		}
	}
}

void ACCDPlayerController::HandleTargetDeath(AController* Killer)
{
	// 3인칭 전환 및 관전 시작
	ACCDCharacter* MyChar = Cast<ACCDCharacter>(GetPawn());
	if (MyChar)
	{
		if (MyChar->GetViewComp()->GetIsFirstPerson())
		{
			MyChar->ToggleView(); // 강제 3인칭
		}
		
		// 추후 Chaos Destruction 연출 추가 예정
		// MyChar->SetLifeSpan(2.0f);
	}

	// 2초 동안 포스트 프로세스 주입 (타이머)
	GetWorldTimerManager().SetTimer(PostProcessTimer, this, &ACCDPlayerController::UpdateDeathVisuals, 0.02f, true);
	
	// 최초 관전 대상 탐색
	SpectateNextPlayer();
}
