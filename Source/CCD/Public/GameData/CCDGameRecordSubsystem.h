// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CCDSaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CCDGameRecordSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API UCCDGameRecordSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/** 새로운 세션 생성 또는 기존 세션 로드 */
	void CreateNewSession(int32 SlotIndex, FString SessionName);
	void LoadSession(int32 SlotIndex);

	/** 현재 맵 클리어 기록 및 자동 저장 */
	void RecordMapClear(FString MapPath);
	
	/** 엔딩 조건 확인 */
	bool AreAllMapsCleared(const TArray<FString>& RequiredMaps) const;

	/** 디스크에 저장 및 로드 */
	void SaveGameProgress();
	void LoadGameProgress();

	/** UI용: 모든 세션 목록 가져오기 */
	const TMap<int32, FSessionData>* GetSessionList() const;

private:
	int32 CurrentSlotIndex = -1;
	FSessionData CurrentSessionData;
	const FString SaveSlotName = TEXT("CCD_GlobalSave");
};
