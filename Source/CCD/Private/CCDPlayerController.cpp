
#include "CCDPlayerController.h"

#include "CCDCharacter.h"
#include "CCDGameMode.h"
#include "CCDPlayerCameraManager.h"
#include "CCDSpectator.h"
#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Widget/SpectatorWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/Scene.h"

ACCDPlayerController::ACCDPlayerController()
{
	PlayerCameraManagerClass = ACCDPlayerCameraManager::StaticClass();
}
void ACCDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Input
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	
	// UI
	if (IsLocalController() && MainWidgetClass)
	{
		MainWidgetInstance = CreateWidget<UUserWidget>(this, MainWidgetClass);
		if (MainWidgetInstance)
		{
			MainWidgetInstance->AddToViewport();
		}
	}
}

/** --- Input --- */
void ACCDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACCDPlayerController::Input_Move);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACCDPlayerController::Input_Look);
		EnhancedInput->BindAction(ChangeTargetLeftAction, ETriggerEvent::Started, this, &ACCDPlayerController::Input_ChangeTargetLeft);
		EnhancedInput->BindAction(ChangeTargetRightAction, ETriggerEvent::Started, this, &ACCDPlayerController::Input_ChangeTargetRight);
	}
}
void ACCDPlayerController::Input_Move(const FInputActionValue& Value)
{
	const FVector2D MoveVector = Value.Get<FVector2D>();

	if (APawn* ControlledPawn = GetPawn())
	{
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		ControlledPawn->AddMovementInput(ForwardDirection, MoveVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, MoveVector.X);
	}
}
void ACCDPlayerController::Input_Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (LookAxisVector.X != 0.0f || LookAxisVector.Y != 0.0f)
	{
		// 마우스 입력을 컨트롤러 회전에 반영
		AddYawInput(LookAxisVector.X);
		AddPitchInput(LookAxisVector.Y);
	}
}
void ACCDPlayerController::Input_ChangeTargetLeft(const FInputActionValue& Value)
{
	const ACCDCharacter* MyCharacter = Cast<ACCDCharacter>(GetPawn());
	if (MyCharacter && MyCharacter->IsDead())
	{
		SpectateNextPlayer(true);
	}
}
void ACCDPlayerController::Input_ChangeTargetRight(const FInputActionValue& Value)
{
	const ACCDCharacter* MyCharacter = Cast<ACCDCharacter>(GetPawn());
	if (MyCharacter && MyCharacter->IsDead())
	{
		SpectateNextPlayer(false);
	}
}

/** --- Spectate --- */
ACCDCharacter* ACCDPlayerController::GetCurrentSpectateTarget() const
{
	return SpectateCandidates.IsValidIndex(CurrentSpectateIndex) 
			? SpectateCandidates[CurrentSpectateIndex] : nullptr;
}
void ACCDPlayerController::SpectateNextPlayer(bool bForward)
{
	// 1. 인스턴스가 없다면 생성 (최초 1회)
	if (!SpectatorInstance && SpectatorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpectatorInstance = GetWorld()->SpawnActor<ACCDSpectator>(SpectatorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}
	
	// 월드의 모든 캐릭터를 찾아 후보 리스트 갱신
	SpectateCandidates.Empty();
	for (TActorIterator<ACCDCharacter> It(GetWorld()); It; ++It)
	{
		if (*It == GetPawn()) SpectateCandidates.Insert(*It, 0);
		else SpectateCandidates.Add(*It);
	}
	if (SpectateCandidates.Num() == 0) return;
	
	// 방향에 따라 인덱스 조정
	if (bForward) CurrentSpectateIndex = (CurrentSpectateIndex + 1) % SpectateCandidates.Num();
	else CurrentSpectateIndex = (CurrentSpectateIndex - 1 + SpectateCandidates.Num()) % SpectateCandidates.Num();
	
	UE_LOG(LogTemp, Warning, TEXT("SpectateNextPlayer: CurrentSpectateIndex = %d"), CurrentSpectateIndex);
		
	// 리스트 순회 후
	if (ACCDCharacter* Target = SpectateCandidates[CurrentSpectateIndex])
	{
		if (SpectatorInstance)
		{
			SpectatorInstance->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			SpectatorInstance->FollowTarget(Target);
			SetViewTarget(SpectatorInstance);
			
			// 관전자 UI 업데이트
			UpdateSpectatorWidget(Target);
		}
	}
}
void ACCDPlayerController::UpdateRotation(float DeltaTime)
{
	Super::UpdateRotation(DeltaTime);
	
	// 마우스 회전값을 관전자 액터에게 전달
	if (SpectatorInstance)
	{
		SpectatorInstance->UpdateCameraRotation(GetControlRotation());
	}
}

/** --- Death --- */
void ACCDPlayerController::ApplyDeath_Implementation(bool bIsDead)
{
	if (ACCDPlayerCameraManager* CCDCamManager = Cast<ACCDPlayerCameraManager>(PlayerCameraManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyDeath"));
		
		if (bIsDead)
		{
			CCDCamManager->SetDeathEffect(true);
			
			SwitchToSpectatorUI();
			SpectateNextPlayer(true);
		}
	}
}

/** --- UI --- */
void ACCDPlayerController::SwitchToSpectatorUI()
{
	if (!IsLocalController()) return;
	
	// 기존 일반 HUD 제거
	if (MainWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchToSpectatorUI 1"));
		MainWidgetInstance->RemoveFromParent();
		MainWidgetInstance = nullptr;
	}
	
	// 관전자 전용 UI 생성 및 표시
	if (SpectatorWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchToSpectatorUI 2"));

		// 인스턴스가 없으면 새로 생성
		if (!SpectatorWidgetInstance || !IsValid(SpectatorWidgetInstance))
		{
			SpectatorWidgetInstance = CreateWidget<USpectatorWidget>(this, SpectatorWidgetClass);
		}
		// 인스턴스가 있고 아직 뷰포트에 없으면 추가
		if (SpectatorWidgetInstance && !SpectatorWidgetInstance->IsInViewport())
		{
			SpectatorWidgetInstance->AddToViewport();
		}
	}
}
void ACCDPlayerController::SwitchToMainUI()
{
	if (!IsLocalController()) return;

	// 관전자 UI 제거
	if (SpectatorWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchToMainUI 1"));
		SpectatorWidgetInstance->RemoveFromParent();
		SpectatorWidgetInstance = nullptr;
	}

	// 기존 메인 HUD 복구
	if (MainWidgetClass && !MainWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchToMainUI 2"));
		MainWidgetInstance = CreateWidget<UUserWidget>(this, MainWidgetClass);
		if (MainWidgetInstance)
		{
			MainWidgetInstance->AddToViewport();
		}
	}
}
void ACCDPlayerController::UpdateSpectatorWidget(TObjectPtr<ACCDCharacter> Target)
{
	if (SpectatorWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s : UpdateSpectatorWidget"), *GetPawn()->GetName());
		SpectatorWidgetInstance->UpdateSpectatorInfo(Target);
	}
}

/** --- Respawn --- */
void ACCDPlayerController::ResetPlayerController(APawn* NewPawn)
{
	if (!NewPawn) return;
	
	if (ACCDPlayerCameraManager* MyCamManager = Cast<ACCDPlayerCameraManager>(PlayerCameraManager))
	{
		MyCamManager->SetDeathEffect(false);
	}
	SetViewTarget(NewPawn);
	SwitchToMainUI();
			
	if (SpectatorInstance)
	{
		SpectatorInstance->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SpectatorInstance->Destroy();
		SpectatorInstance = nullptr;
		SpectateCandidates.Empty();
		CurrentSpectateIndex = -1;
	}
}