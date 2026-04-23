
#include "Widget/TabletWidget.h"

#include "GameData/MapDataRow.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

FString UTabletWidget::GetMapDisplayNameFromAsset() const
{
	if (!MapDataTable) return TEXT("No Table");
	
	// 1. 현재 로드된 레벨의 이름을 가져온다
	FString MapName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);

	// 2. 데이터 테이블의 모든 행을 순회한다
	static const FString ContextString(TEXT("MapLookupContext"));
	TArray<FMapInfoRow*> AllRows;
	MapDataTable->GetAllRows<FMapInfoRow>(ContextString, AllRows);

	for (FMapInfoRow* Row : AllRows)
	{
		if (Row && !Row->MapAsset.IsNull())
		{
			// 3. 테이블에 등록된 에셋 이름과 현재 맵 이름이 일치하는지 확인
			if (Row->MapAsset.GetAssetName().Equals(MapName, ESearchCase::IgnoreCase))
			{
				return Row->MapDisplayName.ToString();
			}
		}
	}

	return TEXT("Unknown Area");
}

FString UTabletWidget::GetCurrentCleanMapName() const
{
	// '/Game/Maps/Level1' 대신 'Level1'만 가져옴
	FString MapName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	return MapName;
}

FString UTabletWidget::GetGameNetModeText() const
{
	if (UWorld* World = GetWorld())
	{
		// 월드의 NetMode를 확인합니다.
		ENetMode NetMode = World->GetNetMode();
        
		// NM_Standalone: 완전한 싱글 플레이
		// NM_ListenServer: 방장으로서 멀티 플레이 중
		// NM_Client: 게스트로서 멀티 플레이 중
		if (NetMode == NM_Standalone)
		{
			return TEXT("SINGLE");
		}
		else
		{
			// 접속자 수까지 표시
			if (AGameStateBase* GS = World->GetGameState())
			{
				int32 Count = GS->PlayerArray.Num();
				return FString::Printf(TEXT("MULTI (%d Players)"), Count);
			}
			return TEXT("MULTI");
		}
	}
	return TEXT("UNKNOWN");
}