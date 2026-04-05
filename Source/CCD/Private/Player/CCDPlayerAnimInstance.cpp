
#include "Player/CCDPlayerAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/CCDCharacter.h"

void UCCDPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 애니메이션 인스턴스가 소유한 캐릭터를 가져옵니다.
	if (APawn* OwningPawn = TryGetPawnOwner())
	{
		OwnerCharacter = Cast<ACCDCharacter>(OwningPawn);
	}
}

void UCCDPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!OwnerCharacter) return;

	// ----- Character -> Speed(X, Y) -----
	const FVector Velocity = OwnerCharacter->GetVelocity();
	const FRotator Rotation = OwnerCharacter->GetActorRotation();
	const FVector LocalVelocity = Rotation.UnrotateVector(Velocity);
	
	ForwardSpeed = LocalVelocity.X;
	RightSpeed = LocalVelocity.Y;
	
	// UE_LOG(LogTemp, Log, TEXT("Forward: %f, Right: %f, Total: %f"), ForwardSpeed, RightSpeed, Velocity.Size());

	// ----- Character -> Equipment State -----
	EquipmentState = OwnerCharacter->GetEquipmentComp()->GetEquipmentState();
	
	// ----- Is Falling -----
	bIsFalling = OwnerCharacter->GetCharacterMovement()->IsFalling();
	
	// ----- Character -> Is Emoting -----
	// 움직임이 감지될 경우 이모트 상태를 자동으로 해제하도록 처리 (예: 이동 시작 시 이모트 취소)
	bIsEmoting = OwnerCharacter->GetIsEmoting();
	if (Velocity.Size() > 0.1f && bIsEmoting)
	{
		bIsEmoting = false;
		OwnerCharacter->SetIsEmoting(false); // 캐릭터의 이모트 상태도 동기화하여 해제
		OwnerCharacter->Server_StopMontage();
	}
	
	// ----- Character -> Is Grabbing -----
	bIsGrabbing = OwnerCharacter->GetIsGrabbed();
}