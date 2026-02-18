
#include "Component/CCD_EquipmentComponent.h"
#include "CCDCharacter.h"
//#include "Component/CCD_ScannerComponent.h"
#include "Component/ProgressComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Widget/ScannerWidget.h"

UCCD_EquipmentComponent::UCCD_EquipmentComponent()
{
	SetIsReplicatedByDefault(true);
	
	//ScannerTool = CreateDefaultSubobject<UCCD_ScannerComponent>(TEXT("ScannerTool"));
}


void UCCD_EquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACCDCharacter>(GetOwner());
	
	// 소유자의 메시들을 찾아 참조 저장 (캐릭터 생성자에서 만든 메시들)
	if (OwnerCharacter)
	{
		MopMesh = OwnerCharacter->GetMopMesh(); 
		HandleEquipmentEffects(EquipmentState);
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
	/*if (!OwnerCharacter) return;
	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && AnimInstance->Montage_IsPlaying(OwnerCharacter->GetEquipMontage())) return;
	
	// 비-재생 중(중도 참가자 등)일 때의 최종 소켓 확정
	switch (NewState)
	{
	case ECCD_EquipmentState::EES_Hands:
		MopMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Back"));
		ScannerTool->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hip"));
		break;

	case ECCD_EquipmentState::EES_Scanner:
		ScannerTool->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hand"));
		MopMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Back"));
		break;

	case ECCD_EquipmentState::EES_Mop:
		MopMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Hand"));
		ScannerTool->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hip"));
		break;
	}*/
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
	/*if (!OwnerCharacter) return;
	if (!OwnerCharacter->HasAuthority()) return;

	if (OwnerCharacter->GetIsUnequipping()) 
	{
		MopMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Back"));
		ScannerTool->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hip"));
		EquipmentState = ECCD_EquipmentState::EES_Hands;
	}
	else
	{
		switch(PendingEquipmentState)
		{
		case ECCD_EquipmentState::EES_Scanner:
			ScannerTool->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ScannerSocket_Hand"));
			break;

		case ECCD_EquipmentState::EES_Mop:
			MopMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MopSocket_Hand"));
			break;
			
		default: break;
		}
		EquipmentState = PendingEquipmentState;
	}*/
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

void UCCD_EquipmentComponent::ExcuteActiveEquipment() const
{/*
	if (!OwnerCharacter || GetEquipmentState() == ECCD_EquipmentState::EES_Hands) return;

	switch(GetEquipmentState())
	{
	case ECCD_EquipmentState::EES_Mop:
		//OwnerCharacter->Server_PlayActionOfMop();
		break;
		
	case ECCD_EquipmentState::EES_Scanner:
		ScannerTool->UpdateScanner();
		break;
		
	default: 
		break;
	}*/
}
