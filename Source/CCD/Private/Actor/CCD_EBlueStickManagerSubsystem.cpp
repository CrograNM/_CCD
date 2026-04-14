
#include "Actor/CCD_EBlueStickManagerSubsystem.h"

#include "Actor/CCD_EBlueStick.h"
#include "Kismet/KismetMaterialLibrary.h"

void UCCD_EBlueStickManagerSubsystem::RegisterStick(ACCD_EBlueStick* InStick)
{
	if (InStick)
	{
		ActiveSticks.AddUnique(InStick);
		
		if (!UVLightMPC)
		{
			// MPC는 블루스틱이 처음 등록될 때 월드에서 찾아서 캐싱
			UVLightMPC = ActiveSticks[0]->GetUVLightMPC();
			if (!UVLightMPC)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to load Material Parameter Collection for Blue Stick Effect!"));
			}
		}
	}
}

void UCCD_EBlueStickManagerSubsystem::UnregisterStick(ACCD_EBlueStick* InStick)
{
	if (InStick) ActiveSticks.Remove(InStick);
}

void UCCD_EBlueStickManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCCD_EBlueStickManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UCCD_EBlueStickManagerSubsystem::Tick(float DeltaTime)
{
	if (!UVLightMPC) return;

	int32 Count = 0;
	for (int32 i = 0; i < ActiveSticks.Num() && Count < MAX_STICK_COUNT; ++i)
	{
		if (!ActiveSticks[i].IsValid()) continue;
		
		FName PosParamName = *FString::Printf(TEXT("StickPos_%d"), Count);
		FName IntensityParamName = *FString::Printf(TEXT("StickIntensity_%d"), Count);
		Count++;
		
		if (ActiveSticks[i]->IsLEDOn())
		{
			UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), UVLightMPC, PosParamName, FLinearColor(ActiveSticks[i]->GetActorLocation()));
			UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), UVLightMPC, IntensityParamName, 1.0f); // 켜진 스틱 : 1, 꺼진 스틱 : 0으로 제어
		}
		else
		{
			UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), UVLightMPC, PosParamName, FLinearColor(ActiveSticks[i]->GetActorLocation()));
			UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), UVLightMPC, IntensityParamName, 0.0f);
		}
	}

	// UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), UVLightMPC, TEXT("ActiveCount"), (float)Count);
}
