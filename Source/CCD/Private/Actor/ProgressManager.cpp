// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ProgressManager.h"

#include "CCDGameMode.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AProgressManager::AProgressManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AProgressManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProgressManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProgressManager::UpdateProgress()
{
	// 목표치 달성 확인
	if (CurrentProgress >= MaxProgress)
	{
		ACCDGameMode* GM = Cast<ACCDGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if (GM)
		{
			GM->OnCleaningFinished();
		}
	}
}

void AProgressManager::UpdateUI()
{
}

