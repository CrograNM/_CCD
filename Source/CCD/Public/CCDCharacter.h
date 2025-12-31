// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CCDCharacter.generated.h"

UCLASS()
class CCD_API ACCDCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACCDCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	
protected: // 상속받은 블루프린트나 자식 클래스는 접근 가능
	/** 1인칭 카메라 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FirstPersonCamera;
	class USpringArmComponent;

	/** 물건을 집기 위한 핸들 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	class UPhysicsHandleComponent* PhysicsHandle;

private: // 이 클래스 내부에서만 접근 가능
	float InteractionDistance = 200.f;

};
