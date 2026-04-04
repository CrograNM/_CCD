
#include "Component/CCD_EquipmentComponent.h"
#include "Player/CCDCharacter.h"
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

void UCCD_EquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyAllEquipment(); 
	Super::EndPlay(EndPlayReason);
}

void UCCD_EquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCCD_EquipmentComponent, EquipmentState);
	DOREPLIFETIME(UCCD_EquipmentComponent, PendingEquipmentState);
	DOREPLIFETIME(UCCD_EquipmentComponent, ReplicatedTools);
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
	
	USceneComponent* BestMesh;
	const bool bIsLocal = OwnerCharacter->IsLocallyControlled();
	if (bIsLocal)
	{
		BestMesh = OwnerCharacter->GetMesh1P();
	}
	else
	{
		BestMesh = OwnerCharacter->GetMesh();
	}
	if (!BestMesh || SpawnedToolMap.Num() == 0) return;
	
	for (auto& Elem : SpawnedToolMap)
	{
		ECCD_EquipmentState ToolType = Elem.Key;
		ACCD_EquipActor_Base* ToolActor = Elem.Value;

		if (!ToolActor) continue;

		// 현재 도구가 선택된 상태(NewState)라면 손으로, 아니면 보관 위치로 보냅니다.
		FName TargetSocket = NAME_None;
		if (ToolType == ECCD_EquipmentState::EES_Mop)
		{
			TargetSocket = (ToolType == NewState) ? TEXT("MopSocket_Hand") : TEXT("MopSocket_Back");
		}
		else if (ToolType == ECCD_EquipmentState::EES_Scanner)
		{
			TargetSocket = (ToolType == NewState) ? TEXT("ScannerSocket_Hand") : TEXT("ScannerSocket_Hip");
		}
		if (TargetSocket != NAME_None)
		{
			ToolActor->AttachToComponent(BestMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocket);
			ToolActor->SetEquipmentActive(ToolType == NewState);
		}
	}
}

void UCCD_EquipmentComponent::OnRep_ReplicatedTools()
{
	// 클라이언트: 복제된 배열을 바탕으로 TMap을 채움
	SpawnedToolMap.Empty();
	for (const FEquipToolInfo& Info : ReplicatedTools)
	{
		if (Info.ToolActor)
		{
			SpawnedToolMap.Add(Info.State, Info.ToolActor);
		}
	}
	// 데이터가 들어왔으니 부착 상태 업데이트
	HandleEquipmentEffects(EquipmentState);
}

void UCCD_EquipmentComponent::ProceedToEquip(ECCD_EquipmentState NewState)
{
	if (!OwnerCharacter || !GetOwner()->HasAuthority()) return;
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
}

void UCCD_EquipmentComponent::HandleEquipNotify()
{
	if (!OwnerCharacter) return;
	
	if (GetOwner()->HasAuthority())
	{
		if (OwnerCharacter && OwnerCharacter->GetIsUnequipping())
		{
			EquipmentState = ECCD_EquipmentState::EES_Hands;
		}
		else
		{
			EquipmentState = PendingEquipmentState;
		}
        
		// 서버에서도 부착 상태 업데이트
		HandleEquipmentEffects(EquipmentState);
	}
	
	// 클라이언트라면? 
	if (!GetOwner()->HasAuthority() && OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		// 로컬 플레이어는 서버 응답을 기다리지 않고 애니메이션 싱크에 맞춰 미리 부착
		ECCD_EquipmentState PredictState = OwnerCharacter->GetIsUnequipping() ? ECCD_EquipmentState::EES_Hands : PendingEquipmentState;
		HandleEquipmentEffects(PredictState);
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
	if (!GetWorld() || !OwnerCharacter || !GetOwner()->HasAuthority()) return;
	
	// 설정된 모든 클래스 정보를 순회합니다.
	for (auto& Elem : ToolClassMap)
	{
		ECCD_EquipmentState State = Elem.Key;
		TSubclassOf<ACCD_EquipActor_Base> ToolClass = Elem.Value;

		if (!ToolClass) continue;
		
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
			
			// 복제용 배열에 추가
			FEquipToolInfo Info;
			Info.State = State;
			Info.ToolActor = NewTool;
			ReplicatedTools.Add(Info);
		}
	}
	HandleEquipmentEffects(EquipmentState);
}
