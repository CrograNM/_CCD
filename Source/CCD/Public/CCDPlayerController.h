
#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "CCDPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class ACCDCharacter;
class ACCDSpectator;
class USpectatorWidget;

UCLASS()
class CCD_API ACCDPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ACCDPlayerController();
	virtual void UpdateRotation(float DeltaTime) override;
	
	/** --- Death --- */
	UFUNCTION(Client, Reliable)
	void ApplyDeath(bool bIsDead);	// Fade 처리
	void UpdateSpectatorWidget(TObjectPtr<ACCDCharacter> Target);
	
	// 관전 대상 반환 -> UI에서 사용 예정
	ACCDCharacter* GetCurrentSpectateTarget() const;
	
	/* --- UI --- */
	void SwitchToSpectatorUI(); // UI 교체 
	void SwitchToMainUI();		// UI 복구
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
	/** --- Input --- */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ChangeTargetLeftAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ChangeTargetRightAction;
	
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_ChangeTargetLeft(const FInputActionValue& Value);
	void Input_ChangeTargetRight(const FInputActionValue& Value);
	
	/** --- Spectate --- */
	void SpectateNextPlayer(bool bForward = true);
	UPROPERTY() TArray<ACCDCharacter*> SpectateCandidates;
	int32 CurrentSpectateIndex = -1;
	
	/** --- Spectator --- */
	UPROPERTY(EditAnywhere, Category = "Spectate")
	TSubclassOf<ACCDSpectator> SpectatorClass;
	UPROPERTY()
	TObjectPtr<ACCDSpectator> SpectatorInstance;
	
	/** --- UI --- */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> MainWidgetClass;
	UPROPERTY()
	UUserWidget* MainWidgetInstance;
		
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<USpectatorWidget> SpectatorWidgetClass;
	UPROPERTY()
	TObjectPtr<USpectatorWidget> SpectatorWidgetInstance;
};
