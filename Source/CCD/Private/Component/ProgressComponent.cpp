
#include "Component/ProgressComponent.h"
#include "Actor/ProgressManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UProgressComponent::UProgressComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicatedByDefault(true);
}


void UProgressComponent::BeginPlay()
{
	Super::BeginPlay();

	// 월드에서 ProgressManager 액터를 찾음
	AActor* ManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), AProgressManager::StaticClass());
	ProgressManager = Cast<AProgressManager>(ManagerActor);

	if (ProgressManager && GetOwner()->HasAuthority())
	{
		ProgressManager->AddMaxProgress(ProgressValue);
	}
}

void UProgressComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (GetOwner()->HasAuthority() && ProgressManager && !bIsTaskFinished)
	{
		ProgressManager->AddMaxProgress(-ProgressValue);
		UE_LOG(LogTemp, Warning, TEXT("Progress Rolled Back: Actor %s destroyed before completion."), *GetOwner()->GetName());
	}
}

void UProgressComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UProgressComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UProgressComponent, ProgressValue);
}

// 서버에서 복제된 ProgressValue가 변경될 때 호출 -> 이거 에디터에서 바꾸면 적용 되는지 모르곘네 (추후 확인하기)
void UProgressComponent::OnRep_ProgressValue()
{
}

void UProgressComponent::UpdateProgressValue(float NewValue)
{
	// 1. 오직 서버에서만 점수 수정을 처리함
	if (!GetOwner()->HasAuthority()) return;

	if (ProgressManager)
	{
		// 이전 값과 차이만큼 매니저의 Max치를 보정 (예: 10 -> 15로 변하면 5만큼 더함)
		float Diff = NewValue - ProgressValue;
		ProgressManager->AddMaxProgress(Diff);
	}

	// 2. 값 변경 (이후 복제되어 클라이언트의 OnRep 호출)
	ProgressValue = NewValue;
}

// 액터가 소각되거나 대걸레질이 완료되었을 때 호출
void UProgressComponent::Notify_ProgressOver()
{
	if (!GetOwner()->HasAuthority()) return;
	
	if (ProgressManager)
	{
		bIsTaskFinished = true;
		ProgressManager->AddCurrentProgress(ProgressValue);
	}
}