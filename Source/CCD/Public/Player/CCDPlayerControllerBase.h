// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CCDPlayerControllerBase.generated.h"

/**
 * 
 */
UCLASS()
class CCD_API ACCDPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()
	
public:
	/** 로딩 시작 명령 */
	UFUNCTION(Client, Reliable)
	void Client_StartLoading();
	
	/** 클라이언트 본인의 네트워크를 끊고 개별적으로 엔딩 맵으로 이동하게 하는 RPC */
	UFUNCTION(Client, Reliable)
	void Client_MoveToEndingLocal(const FString& EndingMapPath);
	
protected:
	virtual void OnRep_Pawn() override; // 새 레벨 도착 감지용
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> LoadingWidgetClass;
};
