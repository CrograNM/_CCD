// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_CheckObserved.h"
#include "AIController.h"
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
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (SCP && PlayerPawn)
	{
		UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

		// 1. 시야 판정 업데이트
		bool bObserved = SCP->IsObserved();
		BB->SetValueAsBool(GetSelectedBlackboardKey(), bObserved);

		// 2. 타겟 액터 설정
		BB->SetValueAsObject(TargetActorKey.SelectedKeyName, PlayerPawn);

		// 3. 거리 계산 로직
		float Distance = FVector::Dist(SCP->GetActorLocation(), PlayerPawn->GetActorLocation());
		BB->SetValueAsFloat(DistanceToPlayerKey.SelectedKeyName, Distance);

		// 관찰 중이면 즉시 이동 멈춤
		if (bObserved)
		{
			AIC->StopMovement();
		}
	}
}
