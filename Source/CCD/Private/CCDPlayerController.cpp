
#include "CCDPlayerController.h"

#include "CCDCharacter.h"
#include "CCDPlayerCameraManager.h"
#include "CCDSpectator.h"
#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ACCDPlayerController::ACCDPlayerController()
{
	PlayerCameraManagerClass = ACCDPlayerCameraManager::StaticClass();
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

void ACCDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Input
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	
	// UI
	if (IsLocalController() && HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
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
		SpectateCandidates.Add(*It);
	}
	if (SpectateCandidates.Num() == 0) return;
	
	// 방향에 따라 인덱스 조정
	if (bForward) CurrentSpectateIndex = (CurrentSpectateIndex + 1) % SpectateCandidates.Num();
	else CurrentSpectateIndex = (CurrentSpectateIndex - 1 + SpectateCandidates.Num()) % SpectateCandidates.Num();
	
	// 리스트 순회 후
	if (ACCDCharacter* Target = SpectateCandidates[CurrentSpectateIndex])
	{
		if (SpectatorInstance)
		{
			SpectatorInstance->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			SpectatorInstance->FollowTarget(Target);
			
			// 시점 전환
			SetViewTarget(SpectatorInstance);
			
			const FString Status = Target->IsDead() ? TEXT("사망") : TEXT("생존");
			UE_LOG(LogTemp, Warning, TEXT("관전 대상 : %s [상태: %s]"), *Target->GetName(), *Status);
		}
	}
}

/** --- Death --- */
void ACCDPlayerController::ApplyDeathOverlay_Implementation(bool bIsDark)
{
	if (PlayerCameraManager)
	{
		if (bIsDark)
		{
			// 현재 화면에서 검은색(0,0,0)으로 2초 동안 페이드 아웃
			// bHoldWhenFinished = true : 계속 어두운 상태 유지
			PlayerCameraManager->StartCameraFade(0.0f, 0.8f, 2.0f, FLinearColor::Black, false, true);
		}
		else
		{
			// 다시 밝게 (부활할 때)
			PlayerCameraManager->StopCameraFade();
		}
	}
}
