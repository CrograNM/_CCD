
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

	const FVector Velocity = OwnerCharacter->GetVelocity();
	const FRotator Rotation = OwnerCharacter->GetActorRotation();
	const FVector LocalVelocity = Rotation.UnrotateVector(Velocity);
	
	ForwardSpeed = LocalVelocity.X;
	RightSpeed = LocalVelocity.Y;
	
	UE_LOG(LogTemp, Log, TEXT("Forward: %f, Right: %f, Total: %f"), ForwardSpeed, RightSpeed, Velocity.Size());
}