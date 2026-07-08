// Fill out your copyright notice in the Description page of Project Settings.


#include "GameData/CCDGameRecordSubsystem.h"

#include "GameData/CCDGameState.h"
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
	
	// 서버에서 GameState의 복제 배열 업데이트
	if (ACCDGameState* GS = GetWorld()->GetGameState<ACCDGameState>())
	{
		GS->ReplicatedClearedMapPaths.Empty();
		for (const auto& Pair : CurrentSessionData.ClearedMaps)
		{
			if (Pair.Value) // 클리어된 맵만 추가
			{
				GS->ReplicatedClearedMapPaths.Add(Pair.Key);
			}
		}
	}
		
	SaveGameToDisk(); // 마지막 사용 슬롯 업데이트를 위해 저장
}

void UCCDGameRecordSubsystem::RecordMapClear(FString MapPath)
{
	if (CurrentSlotIndex == -1) return;
	
	// 현재 세션 데이터 업데이트
	CurrentSessionData.ClearedMaps.FindOrAdd(MapPath) = true;
	CurrentSessionData.LastSavedTime = FDateTime::Now();
	
	// 전체 리스트에 반영
	if (CachedSaveGameObject)
	{
		CachedSaveGameObject->SessionSlots.Add(CurrentSlotIndex, CurrentSessionData);
		SaveGameToDisk();
	}
}

void UCCDGameRecordSubsystem::SetMapClearStatusCustom(FString MapPath, bool bCleared)
{
	if (CurrentSlotIndex == -1) return;

	MapPath = UWorld::RemovePIEPrefix(MapPath);
	
	// 임의로 전달받은 참/거짓 플래그에 맞게 맵 상태를 강제 변환
	if (bCleared)
	{
		CurrentSessionData.ClearedMaps.FindOrAdd(MapPath) = true;
	}
	else
	{
		// 초기화 명령인 경우 목록에서 안전하게 제거
		CurrentSessionData.ClearedMaps.Remove(MapPath);
	}
	
	CurrentSessionData.LastSavedTime = FDateTime::Now();
	
	if (CachedSaveGameObject)
	{
		CachedSaveGameObject->SessionSlots.Add(CurrentSlotIndex, CurrentSessionData);
		SaveGameToDisk();
	}
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

bool UCCDGameRecordSubsystem::IsMapCleared(FString MapPath) const
{
	MapPath = UWorld::RemovePIEPrefix(MapPath);
	
	// 1. 호스트(서버/리슨 서버)인 경우: 자신의 세션 데이터(파일 기반)에서 직접 확인
	if (GetWorld() && GetWorld()->GetNetMode() < NM_Client)
	{
		const bool* bCleared = CurrentSessionData.ClearedMaps.Find(MapPath);
		bool bResult = (bCleared && *bCleared);
        
		UE_LOG(LogTemp, Warning, TEXT("[Server] Map '%s' clear status: %s"), *MapPath, bResult ? TEXT("Yes") : TEXT("No"));
		return bResult;
	}

	// 2. 클라이언트인 경우: GameState에 복제된 데이터를 확인
	if (GetWorld())
	{
		if (ACCDGameState* GS = GetWorld()->GetGameState<ACCDGameState>())
		{
			// 서버가 복제해준 배열에 해당 맵 경로가 있는지 확인
			bool bIsPresent = GS->ReplicatedClearedMapPaths.Contains(MapPath);
            
			UE_LOG(LogTemp, Warning, TEXT("[Client] Map '%s' clear status from GameState: %s"), *MapPath, bIsPresent ? TEXT("Yes") : TEXT("No"));
			return bIsPresent;
		}
	}

	return false;
}

TMap<FString, bool> UCCDGameRecordSubsystem::GetCurrentSessionClearedMaps()
{
	return CurrentSessionData.ClearedMaps;
}

