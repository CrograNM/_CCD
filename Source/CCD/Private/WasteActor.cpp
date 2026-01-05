// Fill out your copyright notice in the Description page of Project Settings.


#include "WasteActor.h"

// Sets default values
AWasteActor::AWasteActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWasteActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWasteActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// 인터페이스 함수 구현
void AWasteActor::Interact(AActor* Interactor)
{
	// 여기서 폐기물이 수거되거나 소각로로 이동하는 로직을 작성
	UE_LOG(LogTemp, Warning, TEXT("Waste Interacted!"));
	// Destroy();
}
