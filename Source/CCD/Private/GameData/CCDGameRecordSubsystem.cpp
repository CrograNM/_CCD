// Fill out your copyright notice in the Description page of Project Settings.


#include "GameData/CCDGameRecordSubsystem.h"

#include "Kismet/GameplayStatics.h"

void UCCDGameRecordSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// 게임 시작 시 기존 저장 파일이 있는지 확인하고 로드합니다.
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		CachedSaveGameObject = Cast<UCCDSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	}
	
	// 파일이 없거나 로드에 실패했다면 새로 생성합니다.
	if (!CachedSaveGameObject)
	{
		CachedSaveGameObject = Cast<UCCDSaveGame>(UGameplayStatics::CreateSaveGameObject(UCCDSaveGame::StaticClass()));
	}
}

void UCCDGameRecordSubsystem::CreateNewSession(FString SessionName)
{
	if (!CachedSaveGameObject) return;

	// 1. 새로운 슬롯 인덱스 결정 (현재 개수 + 1)
	// TMap의 Num()을 활용하거나, 더 안전하게는 마지막 인덱스 + 1을 사용합니다.
	int32 NewSlotIndex = 0;
	if (CachedSaveGameObject->SessionSlots.Num() > 0)
	{
		TArray<int32> Keys;
		CachedSaveGameObject->SessionSlots.GetKeys(Keys);
		Keys.Sort();
		NewSlotIndex = Keys.Last() + 1;
	}

	// 2. 세션 데이터 초기화
	FSessionData NewData;
	NewData.SessionName = SessionName;
	NewData.LastSavedTime = FDateTime::Now();
	NewData.ClearedMaps.Empty();

	// 3. 캐시된 객체 및 현재 세션 정보 업데이트
	CachedSaveGameObject->SessionSlots.Add(NewSlotIndex, NewData);
	CachedSaveGameObject->LastUsedSlotIndex = NewSlotIndex;
	
	CurrentSlotIndex = NewSlotIndex;
	CurrentSessionData = NewData;

	// 4. 즉시 디스크 저장
	SaveGameToDisk();
}

void UCCDGameRecordSubsystem::LoadSession(int32 SlotIndex)
{
	if (!CachedSaveGameObject || !CachedSaveGameObject->SessionSlots.Contains(SlotIndex)) return;

	CurrentSlotIndex = SlotIndex;
	CurrentSessionData = CachedSaveGameObject->SessionSlots[SlotIndex];
	CachedSaveGameObject->LastUsedSlotIndex = SlotIndex;
	
	SaveGameToDisk(); // 마지막 사용 슬롯 업데이트를 위해 저장
}

void UCCDGameRecordSubsystem::RecordMapClear(FString MapPath)
{
	// 	if (CurrentSlotIndex == -1) return;
	// 	
	// 	// 현재 세션 데이터 업데이트
	// 	CurrentSessionData.ClearedMaps.FindOrAdd(MapPath) = true;
	// 	CurrentSessionData.LastSavedTime = FDateTime::Now();
	// 	
	// 	// 전체 리스트에 반영
	// 	if (CachedSaveGameObject)
	// 	{
	// 		CachedSaveGameObject->SessionSlots.Add(CurrentSlotIndex, CurrentSessionData);
	// 		SaveGameToDisk();
	// 	}
}

bool UCCDGameRecordSubsystem::AreAllMapsCleared(const TArray<FString>& RequiredMaps) const
{
	//	for (const FString& Map : RequiredMaps)
	//	{
	//		const bool* bCleared = CurrentSessionData.ClearedMaps.Find(Map);
	//		if (!bCleared || !(*bCleared)) return false;
	//	}
	return true;
}

void UCCDGameRecordSubsystem::SaveGameToDisk()
{
	if (CachedSaveGameObject)
	{
		UGameplayStatics::SaveGameToSlot(CachedSaveGameObject, SaveSlotName, 0);
	}
}

TMap<int32, FSessionData> UCCDGameRecordSubsystem::GetSessionList() const
{
	if (CachedSaveGameObject)
	{
		return CachedSaveGameObject->SessionSlots;
	}
	return TMap<int32, FSessionData>();
}
