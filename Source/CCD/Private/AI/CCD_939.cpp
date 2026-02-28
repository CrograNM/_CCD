// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CCD_939.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ACCD_939::ACCD_939()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
}

// Called when the game starts or when spawned
void ACCD_939::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACCD_939::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACCD_939::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACCD_939::SetMovementState(bool bIsChasing)
{
	GetCharacterMovement()->MaxWalkSpeed = bIsChasing ? ChaseSpeed : PatrolSpeed;
}