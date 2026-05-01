// Fill out your copyright notice in the Description page of Project Settings.


#include "GameData/CCDGameRecordSubsystem.h"

void UCCDGameRecordSubsystem::CreateNewSession(int32 SlotIndex, FString SessionName)
{
}

void UCCDGameRecordSubsystem::LoadSession(int32 SlotIndex)
{
}

void UCCDGameRecordSubsystem::RecordMapClear(FString MapPath)
{
}

bool UCCDGameRecordSubsystem::AreAllMapsCleared(const TArray<FString>& RequiredMaps) const
{
	return false;
}

void UCCDGameRecordSubsystem::SaveGameProgress()
{
}

void UCCDGameRecordSubsystem::LoadGameProgress()
{
}

const TMap<int32, FSessionData>* UCCDGameRecordSubsystem::GetSessionList() const
{
	return nullptr;
}
