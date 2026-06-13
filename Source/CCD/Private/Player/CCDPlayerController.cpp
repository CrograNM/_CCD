
#include "Player/CCDPlayerController.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "Player/CCDCharacter.h"
#include "GameData/CCDGameMode.h"
#include "Player/CCDPlayerCameraManager.h"
#include "Player/CCDSpectator.h"
#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Actor/ProgressManager.h"
#include "Actor/SharedLivesManager.h"
#include "Widget/SpectatorWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "Component/CCD_InteractionComponent.h"
#include "Component/CCD_StatComponent.h"
#include "Component/ProgressComponent.h"
#include "Engine/Scene.h"
#include "GameData/CCDGameInstance.h"
#include "GameData/CCDPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/CCD_MainWidget.h"
#include "Widget/EyeCooldownWidget.h"
#include "Widget/NoiseWidget.h"
#include "Widget/StaminaWidget.h"

ACCDPlayerController::ACCDPlayerController()
{
	PlayerCameraManagerClass = ACCDPlayerCameraManager::StaticClass();
}

void ACCDPlayerController::Server_SetInitialPlayerName_Implementation(const FString& InName)
{
	if (ACCDPlayerState* PS = GetPlayerState<ACCDPlayerState>())
	{
		// "None"이거나 비어있으면 기본 엔진 이름 사용, 아니면 커스텀 이름 사용
		if ((InName.IsEmpty() || InName == TEXT("None")))
		{
			// 기본 엔진 이름을 가져와서 설정
			PS->CustomName = PS->GetPlayerName();
			UE_LOG(LogTemp, Log, TEXT("Server_SetInitialPlayerName: Using default name '%s'"), *PS->CustomName);
		}
		else
		{
			PS->CustomName = InName;
		}
	}
}

void ACCDPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority()) 
		SwitchToMainUI();
	
	// 초기 플레이어 설정
	if (IsLocalController())
	{
		if (UCCDGameInstance* GI = GetGameInstance<UCCDGameInstance>())
		{
			FString MySavedName = GI->GetSavedName();
			Server_SetInitialPlayerName(MySavedName);
		}
		
		MyCharacter = Cast<ACCDCharacter>(GetPawn());
		
		if (PlayerCameraManager)
		{
			LastCameraRotation = PlayerCameraManager->GetCameraRotation();
		}
	}
}
void ACCDPlayerController::OnRep_Pawn()
{
	// 클라이언트 측에서 Pawn이 새로 복제되어 들어올 때마다 호출
	
	Super::OnRep_Pawn();
    
	// Pawn이 복제되어 들어온 이 시점에 UI 바인딩을 수행하는 것이 가장 안전
	if (GetPawn())
	{
		SwitchToMainUI(); // 내부에서 BindUIWithPawn 호출
	}
}

void ACCDPlayerController::PlayerTick(float DeltaTime)
{
	if (!IsLocalController()) return;
	Super::PlayerTick(DeltaTime);

	// 매 프레임 입력 모드 및 카메라 이동에 발맞춰 HUD 밀림/복귀 연산 수행
	UpdateUISway(DeltaTime);
}
void ACCDPlayerController::UpdateUISway(float DeltaTime)
{
	// 화면에 인게임 UI 메인 위젯이 생성되어 띄워져 있을 때만 계산 작동
	if (!MainWidgetInstance || !PlayerCameraManager) return;

	FRotator CurrentCamRot = PlayerCameraManager->GetCameraRotation();
	
	FRotator DeltaRot = (CurrentCamRot - LastCameraRotation).GetNormalized();

	// 카메라가 오른쪽(Yaw+)으로 회전할 때 UI는 왼쪽(X-)으로 밀림
	FVector2D TargetOffset;
	TargetOffset.X = -DeltaRot.Yaw * UISwaySensitivity;
	TargetOffset.Y = DeltaRot.Pitch * UISwaySensitivity;

	// 과도하게 휙 돌렸을 때 UI가 찢어지는 문제를 막기 위해 오프셋 최대 반경 클램핑 처리
	TargetOffset.X = FMath::Clamp(TargetOffset.X, -MaxUISwayOffset, MaxUISwayOffset);
	TargetOffset.Y = FMath::Clamp(TargetOffset.Y, -MaxUISwayOffset, MaxUISwayOffset);

	// 현재 위치에서 목표 위치(마우스 정지 시 자동으로 0,0)로 부드러운 완충 원복
	CurrentUISwayOffset.X = FMath::FInterpTo(CurrentUISwayOffset.X, TargetOffset.X, DeltaTime, UISwaySpeed);
	CurrentUISwayOffset.Y = FMath::FInterpTo(CurrentUISwayOffset.Y, TargetOffset.Y, DeltaTime, UISwaySpeed);

	// 최상단 위젯 레이어 자체의 Render Transform
	MainWidgetInstance->SetRenderTranslation(CurrentUISwayOffset);
	
	LastCameraRotation = CurrentCamRot;
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
    
	// 캐릭터가 있고, 상호작용 컴포넌트가 회전 모드라면 시선 회전을 중단
	if (!MyCharacter) return;
	if (MyCharacter->IsRotationMode()) return;

	// 회전 모드가 아닐 때만 기존 시선 회전 로직을 수행
	if (LookAxisVector.X != 0.0f || LookAxisVector.Y != 0.0f)
	{
		AddYawInput(LookAxisVector.X);
		AddPitchInput(LookAxisVector.Y);
	}
}
void ACCDPlayerController::Input_ChangeTargetLeft(const FInputActionValue& Value)
{
	if (MyCharacter && MyCharacter->IsDead())
	{
		SpectateNextPlayer(true);
	}
}
void ACCDPlayerController::Input_ChangeTargetRight(const FInputActionValue& Value)
{
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

/* --- Exec --- */
void ACCDPlayerController::CCD_CleanAll()
{
	Server_CleanAll();
}
void ACCDPlayerController::Server_CleanAll_Implementation()
{
	// 서버 권한 확인 (호스트/서버만 실행 가능하도록 보장)
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;
	
	// 월드 내의 모든 'ProgressValue > 0'인 액터를 찾아 파괴
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* TargetActor = *It;
		if (IsValid(TargetActor) && !TargetActor->IsPendingKillPending())
		{
			if (UProgressComponent* ProgressComp = It->FindComponentByClass<UProgressComponent>())
			{
				if (ProgressComp->ProgressValue > 0.0f)
				{
					It->Destroy();
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("Server Command: CleanAll executed by Admin."));
}
void ACCDPlayerController::CCD_SetLifeCount(int32 NewLives)
{
	Server_SetLifeCount(NewLives);
}
void ACCDPlayerController::Server_SetLifeCount_Implementation(int32 NewLives)
{
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASharedLivesManager::StaticClass());
	if (ASharedLivesManager* LivesManager = Cast<ASharedLivesManager>(FoundActor))
	{
		LivesManager->Server_SetLives(NewLives);
		UE_LOG(LogTemp, Error, TEXT("Server Command: SetLifeCount executed by Admin. New Lives: %d"), NewLives);
	}
}

void ACCDPlayerController::CCD_FreezeAI()
{
	ServerFreezeAI(true);
}

void ACCDPlayerController::CCD_UnfreezeAI()
{
	ServerFreezeAI(false);
}

void ACCDPlayerController::ServerFreezeAI_Implementation(bool bFreeze)
{
	if (!HasAuthority()) return;
	
	for (TActorIterator<AAIController> It(GetWorld()); It; ++It)
	{
		AAIController* AIC = *It;
		if (AIC && AIC->GetBrainComponent())
		{
			if (bFreeze)
			{
				AIC->GetBrainComponent()->StopLogic(TEXT("Cheat Freeze"));
			}
			else
			{
				AIC->GetBrainComponent()->RestartLogic();
			}
		}
	}
}

bool ACCDPlayerController::ServerFreezeAI_Validate(bool bFreeze)
{
	return true;
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

/** --- UI --- */
void ACCDPlayerController::SwitchToSpectatorUI()
{
	if (!IsLocalController()) return;
	
	// 기존 일반 HUD 제거
	if (MainWidgetInstance)
	{
		MainWidgetInstance->RemoveFromParent();
		MainWidgetInstance = nullptr;
	}
	
	// 관전자 전용 UI 생성 및 표시
	if (SpectatorWidgetClass)
	{
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
		SpectatorWidgetInstance->RemoveFromParent();
		SpectatorWidgetInstance = nullptr;
	}

	// 기존 메인 HUD 복구
	if (MainWidgetClass && !MainWidgetInstance)
	{
		MainWidgetInstance = CreateWidget<UCCD_MainWidget>(this, MainWidgetClass);
		if (MainWidgetInstance)
		{
			MainWidgetInstance->AddToViewport();
		}
	}
	
	BindUIWithPawn(GetPawn());
}

void ACCDPlayerController::UpdateSpectatorWidget(TObjectPtr<ACCDCharacter> Target)
{
	if (SpectatorWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s : UpdateSpectatorWidget"), *GetPawn()->GetName());
		SpectatorWidgetInstance->UpdateSpectatorInfo(Target);
	}
}

void ACCDPlayerController::BindUIWithPawn(APawn* InPawn)
{
	if (!InPawn || !MainWidgetInstance) return;

	// 스탯 컴포넌트 바인딩
	if (UCCD_StatComponent* StatComp = InPawn->FindComponentByClass<UCCD_StatComponent>())
	{
		// --- 스태미나 위젯 ---
		if (MainWidgetInstance->WBP_Stamina)
		{
			// 중복 바인딩 방지
			StatComp->OnStaminaChanged.RemoveAll(MainWidgetInstance->WBP_Stamina);
			StatComp->OnStaminaChanged.AddUObject(MainWidgetInstance->WBP_Stamina, &UStaminaWidget::UpdateStamina);
            
			// 초기값 즉시 반영
			MainWidgetInstance->WBP_Stamina->UpdateStamina(StatComp->GetCurrentStamina(), StatComp->GetMaxStamina());
		}
		
		// --- 시야 쿨타임 위젯 ---
		if (MainWidgetInstance->WBP_EyeCooldown)
		{
			StatComp->OnEyeCooldownChanged.RemoveAll(MainWidgetInstance->WBP_EyeCooldown);
			StatComp->OnEyeCooldownChanged.AddUObject(MainWidgetInstance->WBP_EyeCooldown, &UEyeCooldownWidget::UpdateCooldown);
			
			MainWidgetInstance->WBP_EyeCooldown->UpdateCooldown(StatComp->GetEyeCooldown(), StatComp->GetEyeCooldownDuration());
		}
		
		// --- 소음 레벨 위젯 ---
		if (MainWidgetInstance->WBP_Noise)
		{
			StatComp->OnNoiseLevelChanged.RemoveAll(MainWidgetInstance->WBP_Noise);
			StatComp->OnNoiseLevelChanged.AddUObject(MainWidgetInstance->WBP_Noise, &UNoiseWidget::UpdateNoiseLevel);
			
			MainWidgetInstance->WBP_Noise->UpdateNoiseLevel(StatComp->GetNoiseLevel());
		}
	}
}
