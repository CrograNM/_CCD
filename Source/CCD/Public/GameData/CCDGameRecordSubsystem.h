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
	// 서브시스템 초기화 시 데이터를 로드합니다.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	/** 새로운 세션 생성 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void CreateNewSession(FString SessionName);
	
	/** 기존 게임 로드 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void LoadSession(int32 SlotIndex);

	/** 현재 맵 클리어 기록 및 자동 저장 */
	void RecordMapClear(FString MapPath);
	
	void SetMapClearStatusCustom(FString MapPath, bool bCleared);
	
	/** 엔딩 조건 확인 */
	bool AreAllMapsCleared(const TArray<FString>& RequiredMaps) const;

	/** 디스크에 데이터 쓰기 */
	void SaveGameToDisk();

	/** UI용: 모든 세션 목록 가져오기 */
	UFUNCTION(BlueprintPure, Category = "SaveGame")
	TMap<int32, FSessionData> GetSessionList() const;
	
	UFUNCTION(BlueprintCallable, Category = "GameRecord")
	bool IsMapCleared(FString MapPath) const;
	
	
	TMap<FString, bool> GetCurrentSessionClearedMaps();

private:
	// 현재 활성화된 세션 정보
	int32 CurrentSlotIndex = -1;
	FSessionData CurrentSessionData;

	// 전체 세션 리스트를 담고 있는 세이브 객체
	UPROPERTY()
	UCCDSaveGame* CachedSaveGameObject;

	const FString SaveSlotName = TEXT("CCD_GlobalSave");
};
