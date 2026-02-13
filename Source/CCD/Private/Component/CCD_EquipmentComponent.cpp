
#include "Component/CCD_EquipmentComponent.h"
#include "CCDCharacter.h"
#include "Component/ProgressComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Widget/ScannerWidget.h"

UCCD_EquipmentComponent::UCCD_EquipmentComponent()
{
	SetIsReplicatedByDefault(true);
	
	// 3D 위젯 컴포넌트 생성
	ScannerWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("ScannerWidgetComp"));
    
	// 기본 설정: 스캐너 메시의 자식으로 붙이기 위해 초기화 시점에는 미설정 (BeginPlay에서 수행)
	ScannerWidgetComp->SetWidgetSpace(EWidgetSpace::World); // 3D 공간에 배치
	ScannerWidgetComp->SetDrawSize(FVector2D(8.f, 6.f)); // 위젯 크기에 맞게 조절
}


void UCCD_EquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACCDCharacter>(GetOwner());
	
	// 소유자의 메시들을 찾아 참조 저장 (캐릭터 생성자에서 만든 메시들)
	if (OwnerCharacter)
	{
		MopMesh = OwnerCharacter->GetMopMesh(); 
		ScannerMesh = OwnerCharacter->GetScannerMesh();
		
		HandleEquipmentEffects(EquipmentState);
		
		// 1. 위젯 컴포넌트를 스캐너 메쉬의 소켓에 부착
		if (ScannerMesh && ScannerWidgetComp)
		{
			// 스캐너 메쉬의 디스플레이 부분에 미리 만든 소켓 이름 입력
			ScannerWidgetComp->AttachToComponent(ScannerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("ScreenSocket"));
		}
	}
}

void UCCD_EquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCCD_EquipmentComponent, EquipmentState);
}

void UCCD_EquipmentComponent::OnRep_Pollution()
{
	UpdateMopMeshPollution();
}

void UCCD_EquipmentComponent::ScannerUpdate(float Distance) const
{
	if (!ScannerWidget && ScannerWidgetComp)
	{
		const_cast<UCCD_EquipmentComponent*>(this)->ScannerWidget = Cast<UScannerWidget>(ScannerWidgetComp->GetUserWidgetObject());
	}
	
	// 위젯이 유효한지 확인
	if (ScannerWidget) // 여기서 막히는중
	{
		ScannerWidgetComp->SetHiddenInGame(false);
		
		// 위젯 내부의 업데이트 함수 호출
		UE_LOG(LogTemp, Warning, TEXT("[Scanner] Closest Scan Distance: %f"), Distance); 
		ScannerWidget->UpdateDistanceDisplay(Distance);
	}
}

void UCCD_EquipmentComponent::Server_SetEquipmentState_Implementation(ECCD_EquipmentState NewState)
{
	if (EquipmentState == NewState || !OwnerCharacter) return;
	if ( OwnerCharacter->GetIsUnequipping() || OwnerCharacter->GetIsActionInProgress()) return;
	
	PendingEquipmentState = NewState;

	if (EquipmentState != ECCD_EquipmentState::EES_Hands)
	{
		OwnerCharacter->SetIsUnequipping(true);
		
		FName Section = (EquipmentState == ECCD_EquipmentState::EES_Mop) ? TEXT("DrawMop") : TEXT("DrawScanner");
		OwnerCharacter->SetIsActionInProgress(true);	
		OwnerCharacter->Multicast_PlayEquipMontage(Section, -1.2f);
		OwnerCharacter->BindMontageEndedDelegate();
	}
	else
	{
		ProceedToEquip(NewState);
	}
}

void UCCD_EquipmentComponent::OnRep_EquipmentState(ECCD_EquipmentState PreviousState)
{
	HandleEquipmentEffects(EquipmentState);
}

void UCCD_EquipmentComponent::HandleEquipmentEffects(ECCD_EquipmentState NewState)
{
	if (!OwnerCharacter) return;
	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && AnimInstance->Montage_IsPlaying(OwnerCharacter->GetEquipMontage())) return;
	
	// 비-재생 중(중도 참가자 등)일 때의 최종 소켓 확정
	switch (NewState)
	{
	case ECCD_EquipmentState::EES_Hands:
		MopMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Back"));
		ScannerMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hip"));
		break;

	case ECCD_EquipmentState::EES_Scanner:
		ScannerMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hand"));
		MopMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Back"));
		break;

	case ECCD_EquipmentState::EES_Mop:
		MopMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Hand"));
		ScannerMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hip"));
		break;
	}
}

void UCCD_EquipmentComponent::ProceedToEquip(ECCD_EquipmentState NewState)
{
	if (!OwnerCharacter) return;
	OwnerCharacter->SetIsUnequipping(false);

	if (NewState == ECCD_EquipmentState::EES_Hands)
	{
		OwnerCharacter->Multicast_StopMontage();
		HandleEquipmentEffects(NewState);
		return;
	}

	FName Section = (NewState == ECCD_EquipmentState::EES_Mop) ? TEXT("DrawMop") : TEXT("DrawScanner");
	OwnerCharacter->SetIsActionInProgress(true);
	OwnerCharacter->Multicast_PlayEquipMontage(Section, 1.0f);
	OwnerCharacter->BindMontageEndedDelegate();
	HandleEquipmentEffects(NewState);
}

void UCCD_EquipmentComponent::HandleEquipNotify()
{
	if (!OwnerCharacter) return;
	if (!OwnerCharacter->HasAuthority()) return;

	if (OwnerCharacter->GetIsUnequipping()) 
	{
		MopMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Back"));
		ScannerMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hip"));
		EquipmentState = ECCD_EquipmentState::EES_Hands;
	}
	else
	{
		FName Socket = (PendingEquipmentState == ECCD_EquipmentState::EES_Mop) ? TEXT("MopSocket_Hand") : TEXT("ScannerSocket_Hand");
		UStaticMeshComponent* TargetMesh = (PendingEquipmentState == ECCD_EquipmentState::EES_Mop) ? MopMesh : ScannerMesh;
		TargetMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
		EquipmentState = PendingEquipmentState;
	}
}

void UCCD_EquipmentComponent::UpdateMopMeshPollution()
{
	if (!OwnerCharacter) return;
	if (UMaterialInstanceDynamic* MopMaterial = OwnerCharacter->GetMopMaterial())
	{
		// 머티리얼 파라미터 제어 (예: BloodAmount, PoopAmount)
		MopMaterial->SetScalarParameterValue(TEXT("BloodIntensity"), MopPollution_Blood);
		MopMaterial->SetScalarParameterValue(TEXT("ExcrementIntensity"), MopPollution_Excrement);
        
		// 혹은 두 색상을 섞어서 BaseColor 변경
		FLinearColor CleanColor = FLinearColor(0.228f, 0.343f, 0.405f, 1.0f); // 깨끗한 물 색상
		FLinearColor BloodColor = FLinearColor(0.69f, 0.13f, 0.13f, 1.0f); // 핏빛
		FLinearColor PoopColor = FLinearColor(0.0f, 0.5f, 0.0f, 1.0f); // 배설물

		FLinearColor FinalColor = CleanColor;
		FinalColor = FMath::Lerp(FinalColor, BloodColor, MopPollution_Blood);
		FinalColor = FMath::Lerp(FinalColor, PoopColor, MopPollution_Excrement);
        
		MopMaterial->SetVectorParameterValue(TEXT("BaseColor"), FinalColor);
	}
}

float UCCD_EquipmentComponent::GetScanActorDistance() const
{
	if (!OwnerCharacter) return -1.f;
	
	// 모든 ProgressComp를 순회하며 가장 가까운 탐지 가능한 액터와의 거리를 계산
	float ClosestDistance = MaxScanDistance;
	FVector CharacterLocation = OwnerCharacter->GetActorLocation();
	bool bFound = false;
	
	// 1. 모든 UProgressComponent 인스턴스를 순회
	for (TObjectIterator<UProgressComponent> It; It; ++It)
	{
		UProgressComponent* CurrentComp = *It;

		// 2. 현재 월드에 속한 컴포넌트인지 확인 (에디터/다른 월드 제외)
		if (CurrentComp->GetWorld() != GetWorld()) continue;

		// 3. 이미 청소가 완료된(Owner가 없는) 액터는 무시
		AActor* TargetActor = CurrentComp->GetOwner();
		if (!TargetActor || TargetActor == OwnerCharacter) continue;

		// 4. 거리 계산
		float Distance = FVector::Dist(CharacterLocation, TargetActor->GetActorLocation());

		// 5. 최대 탐지 거리 내에 있고, 현재까지 찾은 거리보다 짧으면 갱신
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			bFound = true;
		}
	}
    
	// 탐지된 것이 없다면 MaxScanDistance 혹은 특정 값 반환
	return bFound ? ClosestDistance : -1.f;
}
