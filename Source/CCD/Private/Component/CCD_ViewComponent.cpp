
#include "Component/CCD_ViewComponent.h"
#include "CCDCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UCCD_ViewComponent::UCCD_ViewComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCCD_ViewComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACCDCharacter>(GetOwner());

	if (OwnerCharacter)
	{
		FollowCamera = OwnerCharacter->GetFollowCamera();
		FirstPersonCamera = OwnerCharacter->GetFirstPersonCamera();
		CameraBoom = OwnerCharacter->GetCameraBoom();
		ApplyViewMode(bIsFirstPerson);
	}
}

void UCCD_ViewComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!OwnerCharacter || !FirstPersonCamera) return;

	if (OwnerCharacter->IsLocallyControlled())
	{
		FRotator CurrentRot = FirstPersonCamera->GetRelativeRotation();

		// 임계값 비교 후 서버 전송
		if (!CurrentRot.Equals(LastSentRotation, RotationThreshold))
		{
			Server_SetFirstPersonCameraRotation(CurrentRot);
			Server_SetControlRotation(OwnerCharacter->GetControlRotation());
			LastSentRotation = CurrentRot;
		}
	}
	else
	{
		// 타 클라이언트 캐릭터의 카메라 회전 보간
		FRotator NewRot = FMath::RInterpTo(OwnerCharacter->GetFirstPersonCamera()->GetRelativeRotation(), Rep_FirstPersonCameraRotation, DeltaTime, 30.0f);
		OwnerCharacter->GetFirstPersonCamera()->SetRelativeRotation(NewRot);
	}
}

void UCCD_ViewComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCCD_ViewComponent, bIsFirstPerson);
	DOREPLIFETIME(UCCD_ViewComponent, Rep_FirstPersonCameraRotation);
}

void UCCD_ViewComponent::ToggleView()
{
	bIsFirstPerson = !bIsFirstPerson;
	ApplyViewMode(bIsFirstPerson);
	Server_ToggleView(bIsFirstPerson);
}

void UCCD_ViewComponent::ApplyViewMode(bool bFirstPerson)
{
	if (!OwnerCharacter) return;

	// 시점에 따른 가시성 및 이동 로직 제어
	if (FollowCamera) FollowCamera->SetActive(!bFirstPerson);
	if (FirstPersonCamera) FirstPersonCamera->SetActive(bFirstPerson);
	
	OwnerCharacter->GetMesh()->SetOwnerNoSee(bFirstPerson);

	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	if (MoveComp)
	{
		OwnerCharacter->bUseControllerRotationYaw = bFirstPerson;
		MoveComp->bOrientRotationToMovement = !bFirstPerson;
	}
}

void UCCD_ViewComponent::Server_ToggleView_Implementation(bool bNewIsFirstPerson)
{
	bIsFirstPerson = bNewIsFirstPerson;
	ApplyViewMode(bIsFirstPerson);
}
void UCCD_ViewComponent::Server_SetFirstPersonCameraRotation_Implementation(FRotator NewRotation)
{
	Rep_FirstPersonCameraRotation = NewRotation;
}
void UCCD_ViewComponent::Server_SetControlRotation_Implementation(FRotator NewRotation)
{
	if (OwnerCharacter) 
		OwnerCharacter->SetRemoteControlRotation(NewRotation);
}
void UCCD_ViewComponent::OnRep_IsFirstPerson()
{
	ApplyViewMode(bIsFirstPerson);
}

