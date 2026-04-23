
#include "Widget/TabletWidget.h"

#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

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