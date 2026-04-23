#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "MapDataRow.generated.h" 

USTRUCT(BlueprintType)
struct CCD_API FMapInfoRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	// 맵 에셋 참조 (예: /Game/Maps/Level1.Level1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapData")
	TSoftObjectPtr<UWorld> MapAsset;
	
	// 해당 에셋일 때 UI에 표시할 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapData")
	FText MapDisplayName;

	// 맵 고유 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapData")
	FString MapInternalID;
};
