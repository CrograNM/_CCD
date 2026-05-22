
#pragma once

#include "CoreMinimal.h"
#include "CCDPlayerControllerBase.h"
#include "InputActionValue.h"
#include "CCDPlayerController.generated.h"

class UCCD_MainWidget;
class UInputMappingContext;
class UInputAction;
class UUserWidget;
class ACCDCharacter;
class ACCDSpectator;
class USpectatorWidget;

UCLASS()
class CCD_API ACCDPlayerController : public ACCDPlayerControllerBase
{
	GENERATED_BODY()
	
public:
	ACCDPlayerController();
	virtual void UpdateRotation(float DeltaTime) override;
	virtual void OnRep_Pawn() override;
	
	UFUNCTION(Server, Reliable)
	void Server_SetInitialPlayerName(const FString& InName);
	
	/** --- Death --- */
	UFUNCTION(Client, Reliable)
	void ApplyDeath(bool bIsDead);	// Fade 처리
	void UpdateSpectatorWidget(TObjectPtr<ACCDCharacter> Target);
	
	/** --- Respawn --- */
	void ResetPlayerController(APawn* NewPawn);
	
	/* --- UI --- */
	void SwitchToSpectatorUI(); // UI 교체 
	void SwitchToMainUI();		// UI 복구
	ACCDCharacter* GetCurrentSpectateTarget() const;
	
	/* --- Exec --- */
	UFUNCTION(Exec)
	void CCD_CleanAll();
	
	UFUNCTION(Exec)
	void CCD_SetLifeCount(int32 NewLives);
	
	UFUNCTION(Exec)
	void CCD_FreezeAI();

	UFUNCTION(Exec)
	void CCD_UnfreezeAI();
	
	/** 서버에서 실제 청소 로직을 수행할 RPC */
	UFUNCTION(Server, Reliable)
	void Server_CleanAll();
	
	UFUNCTION(Server, Reliable)
	void Server_SetLifeCount(int32 NewLives);
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
	/** --- Input --- */
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
	TObjectPtr<UCCD_MainWidget> MainWidgetInstance;
		
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<USpectatorWidget> SpectatorWidgetClass;
	UPROPERTY()
	TObjectPtr<USpectatorWidget> SpectatorWidgetInstance = nullptr;
	
	// 위젯 연결
	void BindUIWithPawn(APawn* InPawn);
	
	// FreezeAI를 서버에서 실행되도록
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFreezeAI(bool bFreeze);
	
private:
	float PostProcessAlpha = 0.f;
	
	UPROPERTY()
	TObjectPtr<ACCDCharacter> MyCharacter = nullptr;
};
