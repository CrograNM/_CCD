// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/FlickerLight.h"
#include "Components/SpotLightComponent.h"

AFlickerLight::AFlickerLight()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	RootComponent = SpotLight;
	
	BaseIntensity = 50000.0f;
	AttenuationRadius = 1500.0f;
	OuterConeAngle = 65.0f;
	InnerConeAngle = 40.0f;
}

void AFlickerLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SpotLight)
	{
		float TimeTime = GetWorld()->GetTimeSeconds();
		float NoiseValue = FMath::PerlinNoise1D(TimeTime * Frequency);
		float FinalIntensity = BaseIntensity + (NoiseValue * Amplitude * BaseIntensity);

		if (NoiseValue < NoiseThreshold) { FinalIntensity = 0.0f; }
		
		SpotLight->SetIntensity(FMath::Max(0.0f, FinalIntensity));
		SpotLight->SetAttenuationRadius(AttenuationRadius);
		SpotLight->SetOuterConeAngle(OuterConeAngle);
		SpotLight->SetInnerConeAngle(InnerConeAngle);
		SpotLight->SetIndirectLightingIntensity(IndirectLightingIntensity);
	}
}

void AFlickerLight::BeginPlay()
{
	Super::BeginPlay();
	
}


