// Fill out your copyright notice in the Description page of Project Settings.


#include "ProgressManager.h"


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

void AProgressManager::UpdateUI()
{
}

