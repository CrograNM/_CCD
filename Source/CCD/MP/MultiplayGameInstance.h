// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MultiplayGameInstance.generated.h"


// 검색 결과를 UI에 전달하기 위한 델리게이트 선언 (매개변수: 서버 이름 리스트)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindSessionsCompleteSignature, const TArray<FString>&, SessionNames);


UCLASS()
class CCD_API UMultiplayGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UMultiplayGameInstance();

	// 세션 인터페이스 초기화
	virtual void Init() override;

	// UI에서 호출할 함수들
	UFUNCTION(BlueprintCallable)
	void HostSession(FName SessionName, bool bIsLAN);

	UFUNCTION(BlueprintCallable)
	void FindSessions();

	UFUNCTION(BlueprintCallable)
	void JoinGameSession(int32 SessionIndex);
	
	// UI에서 바인딩할 수 있는 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Multiplayer")
	FOnFindSessionsCompleteSignature OnFindSessionsCompleteEvent;

protected:
	// 세션 인터페이스 포인터
	IOnlineSessionPtr SessionInterface;

	// 세션 검색 결과 저장
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	/** 데리게이트 콜백 함수들 */
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:
	/* 데리게이트 핸들러 */
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
};
