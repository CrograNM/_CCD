
#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "CCDPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class ACCDCharacter;

UCLASS()
class CCD_API ACCDPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ACCDPlayerController();
	
	/** --- Death --- */
	UFUNCTION(Client, Reliable)
	void ApplyDeathOverlay(bool bIsDark);	// Fade 처리
	
	// 관전 대상 반환 -> UI에서 사용 예정
	ACCDCharacter* GetCurrentSpectateTarget() const;
	
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
	UPROPERTY(EditAnywhere, Category = "Input | Spectate")
	TObjectPtr<UInputAction> ChangeTargetLeftAction;
	UPROPERTY(EditAnywhere, Category = "Input | Spectate")
	TObjectPtr<UInputAction> ChangeTargetRightAction;
	
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_ChangeTargetLeft(const FInputActionValue& Value);
	void Input_ChangeTargetRight(const FInputActionValue& Value);
	
	/** --- Spectate --- */
	void SpectateNextPlayer(bool bForward = true);
	UPROPERTY() TArray<ACCDCharacter*> SpectateCandidates;
	int32 CurrentSpectateIndex = -1;
	
	/** --- UI --- */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	UUserWidget* HUDWidgetInstance;
};
