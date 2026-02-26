
#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "CCDPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;

UCLASS()
class CCD_API ACCDPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ACCDPlayerController();
	
	/** --- Death --- */
	UFUNCTION(Client, Reliable)
	void ApplyDeathOverlay(bool bIsDark);	// Fade 처리
	
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
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	
	/** --- UI --- */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	UUserWidget* HUDWidgetInstance;
};
