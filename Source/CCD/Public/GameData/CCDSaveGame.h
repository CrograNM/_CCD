
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CCDSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FSessionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString SessionName;

	// 맵 경로(Key), 클리어 여부(Value)
	UPROPERTY(BlueprintReadWrite)
	TMap<FString, bool> ClearedMaps;	

	UPROPERTY(BlueprintReadWrite)
	FDateTime LastSavedTime;
};

UCLASS()
class CCD_API UCCDSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FString SavedPlayerName;
	
	/** 인덱스(슬롯 번호)별 세션 데이터 관리 */
	UPROPERTY(BlueprintReadWrite)
	TMap<int32, FSessionData> SessionSlots;

	/** 마지막으로 사용한 슬롯 번호 저장 */
	UPROPERTY(BlueprintReadWrite)
	int32 LastUsedSlotIndex = -1;
};
