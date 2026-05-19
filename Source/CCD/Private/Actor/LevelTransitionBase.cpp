// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/LevelTransitionBase.h"

#include "Components/BoxComponent.h"
#include "GameData/CCDGameMode.h"
#include "GameData/CCDGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/CCDCharacter.h"
#include "Player/CCDPlayerController.h"

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
	
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(RootComponent);

	DoorRelativeLocation = FVector::ZeroVector;
	DoorRelativeScale = FVector(1.0f, 1.0f, 1.0f);
	WaitingAreaRelativeLocation = FVector::ZeroVector;
	WaitingAreaRelativeScale = FVector(100.0f, 100.0f, 100.0f);
	
	InteractVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractVolume"));
	InteractVolume->SetupAttachment(RootComponent);
	
	InteractVolume->SetCollisionProfileName(TEXT("UI")); // 또는 "BlockAll"이나 커스텀 채널
	
	InteractVolumeExtent = FVector(50.0f, 50.0f, 50.0f);
	InteractVolumeRelativeLocation = FVector::ZeroVector;
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
	
	if (HasAuthority())
	{
		if (WaitingArea)
		{
			WaitingArea->OnComponentBeginOverlap.AddDynamic(this, &ALevelTransitionBase::OnWaitingAreaOverlapChange);
			WaitingArea->OnComponentEndOverlap.AddDynamic(this, &ALevelTransitionBase::OnWaitingAreaEndOverlap);
		}
	}
	
	OnRep_CanStart();
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
	if (HasAuthority()) {
		bCanStart = IsWaitingAreaFull();
		OnRep_CanStart();
	}
}

void ALevelTransitionBase::OnWaitingAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasAuthority()) {
		bCanStart = IsWaitingAreaFull();
		OnRep_CanStart();
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
	
	Multicast_PlayDoorSound();

	StartLevelTravel();
	// FTimerHandle TravelTimerHandle;
	// GetWorldTimerManager().SetTimer(TravelTimerHandle, this, &ALevelTransitionBase::StartLevelTravel, TravelDelay, false);
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
	
	if (ACCDGameMode* GM = Cast<ACCDGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->TransitionToLevel(NextLevelPath);
	}
	
	if (ACCDPlayerController* PC = Cast<ACCDPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->CCD_FreezeAI();
	}
}

void ALevelTransitionBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 문 메쉬 설정 (에셋, 위치, 크기)
	if (DoorMesh)
	{
		if (DoorMeshAsset) DoorMesh->SetStaticMesh(DoorMeshAsset);
		DoorMesh->SetRelativeLocation(DoorRelativeLocation);
		DoorMesh->SetRelativeScale3D(DoorRelativeScale);
	}

	// Waiting Area(Box Component) 설정
	if (WaitingArea)
	{
		WaitingArea->SetBoxExtent(WaitingAreaRelativeScale);
		WaitingArea->SetRelativeLocation(WaitingAreaRelativeLocation);
	}
	
	if (InteractVolume)
	{
		InteractVolume->SetBoxExtent(InteractVolumeExtent);
		InteractVolume->SetRelativeLocation(InteractVolumeRelativeLocation);
	}
}

void ALevelTransitionBase::OnRep_CanStart()
{
	if (DoorMesh)
	{
		DoorMesh->SetRenderCustomDepth(bCanStart);
		
		DoorMesh->SetCustomDepthStencilValue(200);
	}
}

void ALevelTransitionBase::Multicast_PlayDoorSound_Implementation()
{
	if (DoorOpenSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DoorOpenSound, GetActorLocation());
	}
}