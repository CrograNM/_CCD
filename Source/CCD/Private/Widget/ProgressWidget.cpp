
#include "Widget/ProgressWidget.h"

#include "Actor/ProgressManager.h"
#include "Kismet/GameplayStatics.h"

void UProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 생성되자마자 월드에서 매니저를 찾아 현재 진행도 강제 업데이트
	AActor* ManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), AProgressManager::StaticClass());
	if (AProgressManager* Manager = Cast<AProgressManager>(ManagerActor))
	{
		UpdatePercent(Manager->GetProgressRatio());
	}
}
