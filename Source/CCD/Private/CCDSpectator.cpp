
#include "CCDSpectator.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"


ACCDSpectator::ACCDSpectator()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->bUsePawnControlRotation = false; // 직접 회전시켜줄 것이므로 false
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = true;
	
	SpectatorCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("SpectatorCamera"));
	SpectatorCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

void ACCDSpectator::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACCDSpectator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACCDSpectator::FollowTarget(AActor* Target)
{
	if (Target)
	{
		// 대상에게 부착 (상대 좌표 0,0,0)
		AttachToActor(Target, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

void ACCDSpectator::UpdateCameraRotation(const FRotator& NewRotation)
{
	if (SpringArm)
	{
		SpringArm->SetWorldRotation(NewRotation);
	}
}