// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlickerLight.generated.h"

class USpotLightComponent;

UCLASS()
class CCD_API AFlickerLight : public AActor
{
	GENERATED_BODY()
    
public:    
	AFlickerLight();
	void Tick(float DeltaTime);

protected:
	virtual void BeginPlay() override;
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpotLightComponent* SpotLight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker|Perlin")
	float Frequency = 5.0f; // 얼마나 빠르게 지직거릴 것인가 (주파수)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker|Perlin")
	float Amplitude = 0.2f;  // 노이즈가 밝기에 미치는 영향력 (진폭)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker|Perlin")
	float BaseIntensity = 3000.0f; // 기본 중심 밝기
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker|Perlin")
	float NoiseThreshold = -0.8f; // 이 값보다 노이즈가 낮아지면 완전히 꺼지게 설정 가능
	
	//빛이 도달하는 최대 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker|Settings")
	float AttenuationRadius = 1000.0f; 

	// 빛이 퍼지는 외곽 각도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker|Settings")
	float OuterConeAngle = 60.0f;

	// 빛의 중심부와 외곽 사이의 부드러움
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker|Settings")
	float InnerConeAngle = 30.0f;

	// 간접광 강도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker|Settings")
	float IndirectLightingIntensity = 1.0f;
	
};
