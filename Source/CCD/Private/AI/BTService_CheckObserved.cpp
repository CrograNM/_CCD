// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_CheckObserved.h"
#include "AIController.h"
#include "Player/CCDCharacter.h"
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
	
	if (!BB->GetValueAsBool(TEXT("HasSpottedPlayer")))
	{
		bool b173LookingAtPlayer = false;

		if (ClosestPlayer)
		{
			FVector SCPForward = SCP->GetActorForwardVector();
			
			FVector ToPlayer = ClosestPlayer->GetActorLocation() - SCP->GetActorLocation();
			ToPlayer.Z = 0.0f; 
			ToPlayer.Normalize();
			
			float ViewDot = FVector::DotProduct(SCPForward, ToPlayer);
			const float SCPVisibilityThreshold = 0.7f;
			
			if (ViewDot >= SCPVisibilityThreshold)
			{
				FHitResult SightHit;
				
				FVector TraceStart = SCP->GetActorLocation() + FVector(0.f, 0.f, SCP->BaseEyeHeight);
				FVector TraceEnd = ClosestPlayer->GetActorLocation();
				
				FCollisionQueryParams TraceParams;
				TraceParams.AddIgnoredActor(SCP);      
				TraceParams.AddIgnoredActor(ClosestPlayer);
				
				bool bIsOccluded = GetWorld()->LineTraceSingleByChannel(
					SightHit, 
					TraceStart, 
					TraceEnd, 
					ECC_Visibility, 
					TraceParams
				);
				
				if (!bIsOccluded)
				{
					b173LookingAtPlayer = true;
				}
			}
		}
		
		const float SpottingRange = 1500.0f; 
		
		if (bAnyOneObserved || b173LookingAtPlayer || (ClosestPlayer && MinDistance <= SpottingRange))
		{
			BB->SetValueAsBool(TEXT("HasSpottedPlayer"), true);
			BB->SetValueAsBool(TEXT("CanMove"), true);
			
			UE_LOG(LogTemp, Warning, TEXT("[AI] SCP-173 Chase Activated! (Player Looked: %s, SCP Looked: %s)"), 
				bAnyOneObserved ? TEXT("True") : TEXT("False"), 
				b173LookingAtPlayer ? TEXT("True") : TEXT("False"));
		}
	}

	if (bAnyOneObserved)
	{
		SCP->SetMovementInstant(false);
		AIC->StopMovement();
	}
	else
	{
		if (BB->GetValueAsBool(TEXT("HasSpottedPlayer")))
		{
			SCP->SetMovementInstant(true);
		}
		else
		{
			SCP->SetMovementInstant(false);
			AIC->StopMovement();
		}
	}
}
