
#include "Component/CCD_EquipmentComponent.h"
#include "CCDCharacter.h"
#include "Actor/CCD_EquipActor_Base.h"
#include "Net/UnrealNetwork.h"

UCCD_EquipmentComponent::UCCD_EquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SetIsReplicatedByDefault(true);
}

void UCCD_EquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACCDCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		InitializeEquipment(); // 장비 초기화 실행
	}
}

void UCCD_EquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCCD_EquipmentComponent, EquipmentState);
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
		OwnerCharacter->Multicast_PlayEquipMontage(Section, -1.5f);
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
	
	// 맵에 저장된 모든 액터를 순회하며 상태에 맞춰 재배치합니다.
	for (auto& Elem : SpawnedToolMap)
	{
		ECCD_EquipmentState ToolType = Elem.Key;
		ACCD_EquipActor_Base* ToolActor = Elem.Value;

		if (!ToolActor) continue;

		// 현재 도구가 선택된 상태(NewState)라면 손으로, 아니면 보관 위치로 보냅니다.
		FName TargetSocket;
		if (ToolType == ECCD_EquipmentState::EES_Mop)
		{
			TargetSocket = (ToolType == NewState) ? TEXT("MopSocket_Hand") : TEXT("MopSocket_Back");
		}
		else if (ToolType == ECCD_EquipmentState::EES_Scanner)
		{
			TargetSocket = (ToolType == NewState) ? TEXT("ScannerSocket_Hand") : TEXT("ScannerSocket_Hip");
		}

		ToolActor->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocket);
		ToolActor->SetEquipmentActive(ToolType == NewState);
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
	OwnerCharacter->Multicast_PlayEquipMontage(Section, 1.5f);
	OwnerCharacter->BindMontageEndedDelegate();
	HandleEquipmentEffects(NewState);
}

void UCCD_EquipmentComponent::HandleEquipNotify()
{
	if (!OwnerCharacter) return;
	if (!OwnerCharacter->HasAuthority()) return;

	// 1. 장비를 집어넣는 중(Unequipping)인 경우
	if (OwnerCharacter->GetIsUnequipping()) 
	{
		for (auto& Elem : SpawnedToolMap)
		{
			if (!Elem.Value) continue;

			// 모든 장비를 보관용 소켓으로 이동시킵니다.
			FName StowSocket = (Elem.Key == ECCD_EquipmentState::EES_Mop) ? TEXT("MopSocket_Back") : TEXT("ScannerSocket_Hip");
			Elem.Value->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, StowSocket);
			Elem.Value->SetEquipmentActive(false);
		}
		EquipmentState = ECCD_EquipmentState::EES_Hands; // 상태를 맨손으로 확정
	}
	// 2. 장비를 꺼내는 중(Equipping)인 경우
	else
	{
		if (SpawnedToolMap.Contains(PendingEquipmentState))
		{
			ACCD_EquipActor_Base* TargetTool = SpawnedToolMap[PendingEquipmentState];
			if (TargetTool)
			{
				// 대기 중인 장비(Pending)를 손 소켓으로 이동시킵니다.
				FName HandSocket = (PendingEquipmentState == ECCD_EquipmentState::EES_Mop) ? TEXT("MopSocket_Hand") : TEXT("ScannerSocket_Hand");
				TargetTool->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocket);
				TargetTool->SetEquipmentActive(true);
			}
		}
		EquipmentState = PendingEquipmentState; // 목표 상태로 확정
	}
}

void UCCD_EquipmentComponent::ExecuteActiveEquipment() const
{
	if (SpawnedToolMap.Contains(EquipmentState))
	{
		if (ACCD_EquipActor_Base* ActiveTool = SpawnedToolMap[EquipmentState])
		{
			// UE_LOG(LogTemp, Warning, TEXT("[EquipComp] ExecuteAction"));
			ActiveTool->ExecuteAction();
		}
	}
}

void UCCD_EquipmentComponent::DestroyAllEquipment()
{
	if (!GetOwner()->HasAuthority()) return;
	for (auto& Elem : SpawnedToolMap)
	{
		if (Elem.Value)
		{
			Elem.Value->Destroy(); // 월드에서 장비 액터 삭제
		}
	}
	SpawnedToolMap.Empty();
	EquipmentState = ECCD_EquipmentState::EES_Hands;
}

void UCCD_EquipmentComponent::InitializeEquipment()
{
	if (!GetWorld() || !OwnerCharacter) return;

	// 설정된 모든 클래스 정보를 순회합니다.
	for (auto& Elem : ToolClassMap)
	{
		ECCD_EquipmentState State = Elem.Key;
		TSubclassOf<ACCD_EquipActor_Base> ToolClass = Elem.Value;

		if (!ToolClass) continue;

		// 서버에서만 실제 액터를 스폰합니다.
		if (GetOwner()->HasAuthority())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = OwnerCharacter;
			SpawnParams.Instigator = OwnerCharacter;

			ACCD_EquipActor_Base* NewTool = GetWorld()->SpawnActor<ACCD_EquipActor_Base>(ToolClass, SpawnParams);
			if (NewTool)
			{
				// 장비 액터 초기화 (소유자 전달 등)
				NewTool->InitializeEquipment(OwnerCharacter);

				// 저장소(TMap)에 기록
				SpawnedToolMap.Add(State, NewTool);

				// 초기 위치 설정 (일단 등이나 허리 소켓에 붙여둡니다)
				FName StowSocket = (State == ECCD_EquipmentState::EES_Mop) ? TEXT("MopSocket_Back") : TEXT("ScannerSocket_Hip");
				NewTool->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, StowSocket);
			}
		}
	}
}
