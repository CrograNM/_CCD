
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
	virtual void PlayerTick(float DeltaTime) override;
	
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
	
	UFUNCTION(Exec)
	void CCD_SetMapClear(FString MapName, bool bCleared);
	
	/** 서버에서 실제 청소 로직을 수행할 RPC */
	UFUNCTION(Server, Reliable)
	void Server_CleanAll();
	
	UFUNCTION(Server, Reliable)
	void Server_SetLifeCount(int32 NewLives);
	
	UFUNCTION(Server, Reliable)
	void Server_SetMapClear(const FString& MapName, bool bCleared);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RefreshMapClear(const FString& MapName, bool bCleared);
	
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
	
	// UI Sway 효과 관련 변수
	FRotator LastCameraRotation = FRotator::ZeroRotator; // 이전 프레임 카메라 회전값
    FVector2D CurrentUISwayOffset = FVector2D::ZeroVector; // 현재 누적된 UI 변위 오프셋
    
    UPROPERTY(EditAnywhere, Category = "Design | UI")
    float UISwaySensitivity = 5.0f; // 마우스 회전에 반응하여 UI가 미끄러지는 감도 수치
    
    UPROPERTY(EditAnywhere, Category = "Design | UI")
    float UISwaySpeed = 10.0f; // 마우스를 멈췄을 때 원래 중앙 정위치로 돌아오는 보간 복귀 속도
    
    UPROPERTY(EditAnywhere, Category = "Design | UI")
    float MaxUISwayOffset = 30.0f; // HUD 레이어가 화면 영역을 과도하게 이탈하지 않도록 차단하는 한계 영역(픽셀 단위)
    
    void UpdateUISway(float DeltaTime);
};
