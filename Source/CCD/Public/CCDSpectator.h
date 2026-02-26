
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CCDSpectator.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class CCD_API ACCDSpectator : public AActor
{
	GENERATED_BODY()

public:
	ACCDSpectator();
	void FollowTarget(AActor* Target);
	void UpdateCameraRotation(const FRotator& NewRotation);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> SpectatorCamera;
	
public:
	virtual void Tick(float DeltaTime) override;
};
