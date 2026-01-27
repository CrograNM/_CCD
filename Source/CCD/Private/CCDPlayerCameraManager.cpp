// Fill out your copyright notice in the Description page of Project Settings.


#include "CCDPlayerCameraManager.h"

ACCDPlayerCameraManager::ACCDPlayerCameraManager()
{
	// Pitch 제한 설정 (도 단위)
	ViewPitchMin = -60.0f; // 아래로 
	ViewPitchMax = 45.0f;  // 위로
}