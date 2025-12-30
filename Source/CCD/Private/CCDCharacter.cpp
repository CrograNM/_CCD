// Fill out your copyright notice in the Description page of Project Settings.


#include "CCDCharacter.h"

// Sets default values
ACCDCharacter::ACCDCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACCDCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACCDCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACCDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

