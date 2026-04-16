
#include "Widget/NarrativeWidget.h"
#include "TimerManager.h"

void UNarrativeWidget::InitNarrative(const TArray<FString>& InParagraphs)
{
	if (InParagraphs.Num() == 0) return;

	ParagraphList = InParagraphs;
	CurrentParagraphIndex = 0;
    
	// 첫 번째 문단 시작
	NextParagraph();
}

void UNarrativeWidget::NextParagraph()
{
	// 이미 글자가 나오는 중이라면 로직을 건너뛰기 (나중에 '한번에 보이기'로 확장)
	if (bIsAnimating) return;

	if (ParagraphList.IsValidIndex(CurrentParagraphIndex))
	{
		TargetString = ParagraphList[CurrentParagraphIndex];
		CurrentCharIndex = 0;
		bIsAnimating = true;

		// 텍스트 초기화
		NarrativeText->SetText(FText::GetEmpty());

		// 0.2초 간격으로 타이머 설정
		GetWorld()->GetTimerManager().SetTimer(
			TypewriterTimerHandle,
			this,
			&UNarrativeWidget::PlayTypewriter,
			0.2f,
			true
		);

		CurrentParagraphIndex++;
	}
	else
	{
		// 모든 문단이 끝났을 때의 처리 (예: 타이틀 메뉴로 이동)
		UE_LOG(LogTemp, Warning, TEXT("모든 문단 출력 완료"));
		
		// 위젯 종료
		RemoveFromParent();
	}
}

void UNarrativeWidget::PlayTypewriter()
{
	if (CurrentCharIndex < TargetString.Len())
	{
		CurrentCharIndex++;
		FString DisplayString = TargetString.Left(CurrentCharIndex);
		NarrativeText->SetText(FText::FromString(DisplayString));
	}
	else
	{
		// 현재 문단 출력 완료
		GetWorld()->GetTimerManager().ClearTimer(TypewriterTimerHandle);
		bIsAnimating = false;
	}
}