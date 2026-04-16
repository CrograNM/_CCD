
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

	// 다음 문단으로 넘어가는 함수 (클릭 이벤트 등에 연결)
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void NextParagraph();

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NarrativeText;	// UI에 배치할 TextBlock (이름 매칭 필요)

private:
	void PlayTypewriter();		// 글자를 하나씩 추가하는 핵심 로직

	TArray<FString> ParagraphList;
	int32 CurrentParagraphIndex = 0;
	int32 CurrentCharIndex = 0;

	FTimerHandle TypewriterTimerHandle;
	FString TargetString;

	bool bIsAnimating = false;
};
