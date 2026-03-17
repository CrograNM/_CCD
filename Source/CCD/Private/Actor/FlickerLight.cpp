// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/FlickerLight.h"
#include "TimerManager.h"
#include "Components/PointLightComponent.h"

AFlickerLight::AFlickerLight()
{
	PrimaryActorTick.bCanEverTick = true;

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	RootComponent = PointLight;
}

void AFlickerLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PointLight)
	{
		float TimeTime = GetWorld()->GetTimeSeconds();
		float NoiseValue = FMath::PerlinNoise1D(TimeTime * Frequency);
		
		float FinalIntensity = BaseIntensity + (NoiseValue * Amplitude * BaseIntensity);

		// 임계값(Threshold) 체크: 노이즈가 너무 낮으면 아예 끔
		if (NoiseValue < NoiseThreshold)
		{
			FinalIntensity = 0.0f;
		}

		PointLight->SetIntensity(FMath::Max(0.0f, FinalIntensity));
	}
}

void AFlickerLight::BeginPlay()
{
	Super::BeginPlay();
	
}


