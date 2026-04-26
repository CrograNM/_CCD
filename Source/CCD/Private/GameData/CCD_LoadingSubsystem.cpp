// Fill out your copyright notice in the Description page of Project Settings.


#include "GameData/CCD_LoadingSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "UObject/UObjectGlobals.h"

void UCCD_LoadingSubsystem::ShowLoadingScreen(TSubclassOf<UUserWidget> WidgetClass)
{
	if (WidgetClass && !LoadingWidgetInstance)
	{
		// GameInstance를 아우터로 설정하여 위젯을 생성
		LoadingWidgetInstance = CreateWidget<UUserWidget>(GetGameInstance(), WidgetClass);
		if (LoadingWidgetInstance)
		{
			LoadingWidgetInstance->AddToViewport(999);
		}
	}
}

void UCCD_LoadingSubsystem::HideLoadingScreen()
{
	if (LoadingWidgetInstance)
	{
		LoadingWidgetInstance->RemoveFromParent();
		LoadingWidgetInstance = nullptr;
	}
}

float UCCD_LoadingSubsystem::GetLoadingProgress() const
{
	if (!GetWorld()) return 1.0f;

	const float Progress = GetAsyncLoadPercentage(*GetWorld()->GetOutermost()->GetName());
	return (Progress >= 0.0f) ? Progress / 100.0f : 1.0f;
}

FString UCCD_LoadingSubsystem::GetCleanPathFromSoftObject(TSoftObjectPtr<UObject> SoftObject)
{
	// 소프트 레퍼런스에서 전체 경로를 가져옵니다.
	// 결과 예시: "/Game/Maps/TUWorld.TUWorld"
	FString FullPath = SoftObject.ToString();

	if (FullPath.IsEmpty()) return TEXT("");

	// 점('.') 위치를 찾아 그 앞까지만 반환합니다.
	int32 DotIndex;
	if (FullPath.FindChar('.', DotIndex))
	{
		return FullPath.Left(DotIndex);
	}

	return FullPath;
}
