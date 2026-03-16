// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_CheckObserved.h"
#include "AIController.h"
#include "CCDCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/CCD_173.h"
#include "Kismet/GameplayStatics.h"

UBTService_CheckObserved::UBTService_CheckObserved()
{
	NodeName = TEXT("Check Observation and Distance");
	Interval = 0.01f;
}

void UBTService_CheckObserved::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIC = OwnerComp.GetAIOwner();
	ACCD_173* SCP = Cast<ACCD_173>(AIC->GetPawn());
	if (!SCP || !AIC) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	ACCDCharacter* ClosestPlayer = nullptr;
	float MinDistance = FLT_MAX;
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (ACCDCharacter* PlayerChar = Cast<ACCDCharacter>(PC->GetPawn()))
			{
				if (!PlayerChar->IsDead())
				{
					float Distance = FVector::Dist(SCP->GetActorLocation(), PlayerChar->GetActorLocation());
					if (Distance < MinDistance)
					{
						MinDistance = Distance;
						ClosestPlayer = PlayerChar;
					}
				}
			}
		}
	}
	
	if (ClosestPlayer)
	{
		BB->SetValueAsObject(TargetActorKey.SelectedKeyName, ClosestPlayer);
		BB->SetValueAsFloat(DistanceToPlayerKey.SelectedKeyName, MinDistance);
	}
	
	else
	{
		BB->ClearValue(TargetActorKey.SelectedKeyName);
		BB->SetValueAsBool(GetSelectedBlackboardKey(), false);
		
		AIC->StopMovement();
	}
	
	bool bAnyOneObserved = SCP->IsObserved();
	BB->SetValueAsBool(GetSelectedBlackboardKey(), bAnyOneObserved);

	if (bAnyOneObserved)
	{
		AIC->StopMovement();
	}
}
