// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/LevelTransitionBase.h"

#include "Components/BoxComponent.h"
#include "GameData/CCDGameState.h"
#include "Net/UnrealNetwork.h"
#include "Player/CCDCharacter.h"

// Sets default values
ALevelTransitionBase::ALevelTransitionBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	
	WaitingArea = CreateDefaultSubobject<UBoxComponent>(TEXT("WaitingArea"));
	WaitingArea->SetupAttachment(RootComponent);
	WaitingArea->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	StatusLightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatusLightMesh"));
	StatusLightMesh->SetupAttachment(RootComponent);

}

void ALevelTransitionBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALevelTransitionBase, bCanStart);
	DOREPLIFETIME(ALevelTransitionBase, bIsLoading);
}

// Called when the game starts or when spawned
void ALevelTransitionBase::BeginPlay()
{
	Super::BeginPlay();

	if (StatusLightMesh)
	{
		StatusLightMaterial = StatusLightMesh->CreateAndSetMaterialInstanceDynamic(0);
		OnRep_CanStart();
	}

	if (HasAuthority())
	{
		WaitingArea->OnComponentBeginOverlap.AddDynamic(this, &ALevelTransitionBase::OnWaitingAreaOverlapChange);
		WaitingArea->OnComponentEndOverlap.AddDynamic(this, &ALevelTransitionBase::OnWaitingAreaEndOverlap);
	}
}

bool ALevelTransitionBase::IsWaitingAreaFull() const
{
	if (!WaitingArea || !GetWorld()->GetGameState()) return false;

	int32 TotalPlayers = GetWorld()->GetGameState()->PlayerArray.Num();
	int32 OverlappingPlayers = 0;

	TArray<AActor*> OverlappingActors;
	WaitingArea->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (APawn* Pawn = Cast<APawn>(Actor))
		{
			if (Pawn->IsPlayerControlled()) OverlappingPlayers++;
		}
	}
	return (OverlappingPlayers >= TotalPlayers && TotalPlayers > 0);
}

void ALevelTransitionBase::OnWaitingAreaOverlapChange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		bCanStart = IsWaitingAreaFull();
		OnRep_CanStart();
	}
}

void ALevelTransitionBase::OnWaitingAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasAuthority())
	{
		bCanStart = IsWaitingAreaFull();
		OnRep_CanStart();
	}
}

void ALevelTransitionBase::OnRep_CanStart()
{
	if (StatusLightMaterial)
	{
		// 준비 완료(초록): (0, 5, 0), 대기(빨강): (5, 0, 0)
		FVector Color = bCanStart ? FVector(0.0f, 5.0f, 0.0f) : FVector(5.0f, 0.0f, 0.0f);
		StatusLightMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
	}
}

void ALevelTransitionBase::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || bIsLoading) return;
	
	if (bNeedToCheckProgressOver)
	{
		ACCDGameState* GS = GetWorld()->GetGameState<ACCDGameState>();
		if (!GS || !GS->bIsCleaningFinished) return;
	}
	
	if (!bCanStart || !IsWaitingAreaFull()) return;

	bIsLoading = true;

	FTimerHandle TravelTimerHandle;
	GetWorldTimerManager().SetTimer(TravelTimerHandle, this, &ALevelTransitionBase::StartLevelTravel, TravelDelay, false);
}

void ALevelTransitionBase::StartLevelTravel()
{
	if (!HasAuthority() || NextLevelPath.IsEmpty()) return;
	
	TArray<AActor*> OverlappingActors;
	WaitingArea->GetOverlappingActors(OverlappingActors);
	for (AActor* Actor : OverlappingActors)
	{
		if (ACCDCharacter* Character = Cast<ACCDCharacter>(Actor))
		{
			Character->DestroyAllEquipment();
		}
	}
	
	GetWorld()->ServerTravel(NextLevelPath);
}

