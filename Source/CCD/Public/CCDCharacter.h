// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

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


protected: 
	// - 카메라
	// 현재 1인칭인지 확인하는 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	bool bIsFirstPerson = false;
	
	// 1인칭 카메라 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FirstPersonCamera;
	
	// 3인칭 - 카메라를 캐릭터 뒤에 고정시켜줄 지지대
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* CameraBoom;
	
	// 3인칭 - 실제 플레이어가 보게 될 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

	// - 물리 핸들
	// 물건을 집기 위한 핸들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	class UPhysicsHandleComponent* PhysicsHandle;
	
	// - 상호작용
	// 상호작용 실행 함수
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void PerformInteract();

	// 상호작용 사거리
	UPROPERTY(EditAnywhere, Category = "Design")
	float InteractRange = 300.f;
	
public:
	// BlueprintCallable: 블루프린트에서 이 함수를 호출할 수 있게 합니다.
	// Category: 블루프린트 우클릭 메뉴에서 어떤 카테고리에 보일지 정합니다.
	
	// 시점 전환 함수
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ToggleView();
	

};
