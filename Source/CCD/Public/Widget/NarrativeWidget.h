
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "NarrativeWidget.generated.h"

UCLASS()
class CCD_API UNarrativeWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 초기 문단 배열을 설정하고 재생을 시작하는 함수
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void InitNarrative(const TArray<FString>& InParagraphs);

	// 버전 1: 이전 문단을 지우고 새 문단 출력
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void NextParagraph_Replace();

	// 버전 2: 이전 문단을 유지하고 줄바꿈 후 새 문단 추가
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void NextParagraph_Append();

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NarrativeText;	// UI에 배치할 TextBlock (이름 매칭 필요)

private:
	void StartTypewriter(bool bAppend);
	void PlayTypewriter();		// 글자를 하나씩 추가하는 핵심 로직
	void FinishTypewriterEarly(); // 타이핑 강제 종료 및 문단 완성
	
	TArray<FString> ParagraphList;
	int32 CurrentParagraphIndex = 0;
	int32 CurrentCharIndex = 0;

	FTimerHandle TypewriterTimerHandle;
	FString CurrentTargetParagraph; // 현재 타이핑 중인 문단
	FString AccumulatedText;        // (Append용) 이미 출력이 완료된 이전 전체 텍스트
	
	bool bIsAnimating = false;
	bool bIsAppendMode = false;     // 현재 실행 중인 모드 확인용
};
