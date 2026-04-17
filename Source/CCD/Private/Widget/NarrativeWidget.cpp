
#include "Widget/NarrativeWidget.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UNarrativeWidget::InitNarrative(const TArray<FString>& InParagraphs)
{
	if (InParagraphs.Num() == 0) return;

	ParagraphList = InParagraphs;
	CurrentParagraphIndex = 0;
	AccumulatedText = TEXT("");
}

void UNarrativeWidget::NextParagraph_Replace()
{
	if (bIsAnimating) 
	{
		FinishTypewriterEarly();
		return;
	}

	if (ParagraphList.IsValidIndex(CurrentParagraphIndex))
	{
		bIsAppendMode = false;
		AccumulatedText = TEXT(""); 
		CurrentTargetParagraph = ParagraphList[CurrentParagraphIndex];
		StartTypewriter(false);
	}
	else
	{
		// 더 이상 대사가 없으면 이벤트 호출
		OnNarrativeFinished.Broadcast();
	}
}

void UNarrativeWidget::NextParagraph_Append()
{
	// 글자가 나오는 중이라면 즉시 해당 문단 완성
	if (bIsAnimating)
	{
		FinishTypewriterEarly();
		return;
	}
	if (ParagraphList.IsValidIndex(CurrentParagraphIndex))
	{
		bIsAppendMode = true;
		CurrentTargetParagraph = ParagraphList[CurrentParagraphIndex];

		if (!AccumulatedText.IsEmpty())
		{
			AccumulatedText += TEXT("\n\n");
		}

		StartTypewriter(bIsAppendMode);
	}
	else
	{
		// 모든 문단 출력 완료 시 이벤트 브로드캐스트
		UE_LOG(LogTemp, Log, TEXT("Narrative Finished - Event Broadcasted"));
		OnNarrativeFinished.Broadcast();
	}
}

void UNarrativeWidget::StartTypewriter(bool bAppend)
{
	CurrentCharIndex = 0;
	bIsAnimating = true;

	GetWorld()->GetTimerManager().SetTimer(
		TypewriterTimerHandle,
		this,
		&UNarrativeWidget::PlayTypewriter,
		TypeInterval,
		true
	);

	CurrentParagraphIndex++;
}

void UNarrativeWidget::PlayTypewriter()
{
	if (CurrentCharIndex < CurrentTargetParagraph.Len())
	{
		// 타이핑 사운드 재생
		if (TypeSound)
		{
			UGameplayStatics::PlaySound2D(this, TypeSound);
		}
		
		CurrentCharIndex++;
		FString TypingPart = CurrentTargetParagraph.Left(CurrentCharIndex);
		
		// 누적 텍스트 뒤에 현재 타이핑 부분을 합쳐서 출력
		const FString FullText = AccumulatedText + TypingPart;
		NarrativeText->SetText(FText::FromString(FullText));
	}
	else
	{
		FinishTypewriterEarly();
	}
}

void UNarrativeWidget::FinishTypewriterEarly()
{
	GetWorld()->GetTimerManager().ClearTimer(TypewriterTimerHandle);
	
	// 현재 문단 전체를 화면에 표시
	NarrativeText->SetText(FText::FromString(AccumulatedText + CurrentTargetParagraph));
	
	// 다음 문단을 위해 AccumulatedText 업데이트
	if (bIsAppendMode)
	{
		AccumulatedText += CurrentTargetParagraph;
	}
	
	bIsAnimating = false;
}
