
#include "Component/CCD_InteractionComponent.h"
#include "Player/CCDCharacter.h"
#include "Camera/CameraComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Component/BurnableComponent.h"
#include "Interface/InteractInterface.h"
#include "Actor/WasteActor_Base.h"
#include "Net/UnrealNetwork.h"

UCCD_InteractionComponent::UCCD_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true); // 컴포넌트 복제 활성화
}

void UCCD_InteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACCDCharacter>(GetOwner());
	
	SetComponentTickEnabled(false);
	
	// 소유자 캐릭터가 이미 가지고 있는 PhysicsHandle을 찾아옵니다.
	if (OwnerCharacter)
	{
		PhysicsHandle = OwnerCharacter->FindComponentByClass<UPhysicsHandleComponent>();
	}
}

void UCCD_InteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (GrabbedComponent)
	{
		PhysicsHandleUpdate(DeltaTime);
	}
}

void UCCD_InteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCCD_InteractionComponent, GrabbedComponent);
}

void UCCD_InteractionComponent::AddRotationInput(float Pitch, float Yaw)
{
	float Sensitivity = 2.0f;
	CustomRotationOffset.Pitch -= Pitch * Sensitivity;
	CustomRotationOffset.Yaw += Yaw * Sensitivity;
}

void UCCD_InteractionComponent::PhysicsHandleUpdate(float DeltaTime)
{
	if (!PhysicsHandle || !GrabbedComponent || !OwnerCharacter) return;
    
	float TargetDistance = 200.f; 
	UCameraComponent* PlayerCam = OwnerCharacter->GetFirstPersonCamera();
	FVector RealTargetLocation = PlayerCam->GetComponentLocation() + (PlayerCam->GetForwardVector() * TargetDistance);
	
	FQuat CameraQuat = PlayerCam->GetComponentRotation().Quaternion();
	FQuat RelativeQuat = GrabRelativeRotation.Quaternion();
	FQuat CustomQuat = CustomRotationOffset.Quaternion();
	
	FQuat CombinedQuat = CameraQuat * RelativeQuat * CustomQuat;
	FRotator RealTargetRotation = CombinedQuat.Rotator();
	
	FVector CurrentLocation {};
	FRotator CurrentRotation {};
	PhysicsHandle->GetTargetLocationAndRotation(CurrentLocation, CurrentRotation);
    
	float FollowSpeed = 10.0f; 
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, RealTargetLocation, DeltaTime, FollowSpeed);
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, RealTargetRotation, DeltaTime, FollowSpeed);
    
	PhysicsHandle->SetTargetLocationAndRotation(NewLocation, NewRotation);
}

void UCCD_InteractionComponent::PerformInteract()
{
	Server_PerformInteract();
}

void UCCD_InteractionComponent::ForceRelease()
{
	// 서버에서 실행 중이라면 멀티캐스트로 모두에게 알림
	if (GetOwner()->HasAuthority())
	{
		Multicast_ReleaseObject();
	}
	else
	{
		ReleaseObject_Impl();
	}
}

void UCCD_InteractionComponent::Server_PerformInteract_Implementation()
{
	if (!OwnerCharacter) return;
	if (GrabbedComponent)
	{
		Multicast_ReleaseObject();
		return;
	}

	// 캐릭터의 현재 카메라 위치와 방향을 가져옵니다.
	UCameraComponent* ActiveCam = OwnerCharacter->GetFirstPersonCamera(); 
	FVector TraceStart = ActiveCam->GetComponentLocation();
	FVector TraceEnd = TraceStart + (ActiveCam->GetForwardVector() * InteractRange);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor) return;
		UE_LOG(LogTemp, Warning, TEXT("[Hand] Interacted with : %s"), *HitActor->GetName());
		
		if (UBurnableComponent* BurnComp = HitActor->FindComponentByClass<UBurnableComponent>())
		{
			UE_LOG(LogTemp, Warning, TEXT("[Hand] Target has BurnableComponent!"));
			UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(HitActor->GetRootComponent());
			if (RootPrim)
			{
				if (RootPrim->IsSimulatingPhysics())
				{
					Multicast_GrabObject(RootPrim, HitResult.ImpactPoint);
					UE_LOG(LogTemp, Warning, TEXT("[Hand] Grabbed with : %s"), *HitActor->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("[Hand] Target is NOT simulating physics!"));
				}
			}
			else UE_LOG(LogTemp, Error, TEXT("[Hand] no root primitive component!"));
			IInteractInterface::Execute_Interact(BurnComp, OwnerCharacter);
		}
		else if (HitActor->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Hand] Target does not has BurnableComponent!"));
			IInteractInterface::Execute_Interact(HitActor, OwnerCharacter);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Hand] Target does not implement InteractInterface!"));
		}
	}
}

void UCCD_InteractionComponent::Multicast_GrabObject_Implementation(UPrimitiveComponent* ComponentToGrab, FVector GrabLocation)
{
	GrabObject_Impl(ComponentToGrab, GrabLocation);
}
void UCCD_InteractionComponent::GrabObject_Impl(UPrimitiveComponent* ComponentToGrab, FVector GrabLocation)
{
	if (!OwnerCharacter) return;
	if (!PhysicsHandle || !ComponentToGrab) return;
	OwnerCharacter->SetIsActionInProgress(true); // 상호작용 중 상태 설정
	
	GrabbedComponent = ComponentToGrab;
	
	float CameraYaw = OwnerCharacter->GetFirstPersonCamera()->GetComponentRotation().Yaw;
	float ObjectYaw = GrabbedComponent->GetComponentRotation().Yaw;
	GrabRelativeRotation = FRotator(0.f, ObjectYaw - CameraYaw, 0.f);
	
	GrabbedComponent->SetSimulatePhysics(true);
	GrabbedComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	if (AWasteActor_Base* WasteActor = Cast<AWasteActor_Base>(GrabbedComponent->GetOwner()))
	{
		WasteActor->UpdatePhysicsReplicates(false);
	}
	// 물리 핸들로 잡기 실행
	PhysicsHandle->GrabComponentAtLocationWithRotation(
		ComponentToGrab,
		NAME_None,
		GrabLocation,
		GrabbedComponent->GetComponentRotation()
	);
	
	SetComponentTickEnabled(true);
}

void UCCD_InteractionComponent::Multicast_ReleaseObject_Implementation()
{
	ReleaseObject_Impl();
}
void UCCD_InteractionComponent::ReleaseObject_Impl()
{
	if (!OwnerCharacter) return;
	if (!PhysicsHandle || !GrabbedComponent) return;
	OwnerCharacter->SetIsActionInProgress(false); // 상호작용 중 상태 설정
	
	bIsRotationMode = false;
	CustomRotationOffset = FRotator::ZeroRotator;
	
	PhysicsHandle->ReleaseComponent();
	
	if (GrabbedComponent)
	{
		GrabbedComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

		if (AWasteActor_Base* WasteActor = Cast<AWasteActor_Base>(GrabbedComponent->GetOwner()))
		{
			WasteActor->UpdatePhysicsReplicates(true);
		}
	}
	GrabbedComponent = nullptr;
	
	SetComponentTickEnabled(false);
}

void UCCD_InteractionComponent::SetRotationMode(bool bActive)
{
	if (bActive)
	{
		if (GrabbedComponent == nullptr)
		{
			bIsRotationMode = false;
			return;
		}
		
		CustomRotationOffset = FRotator::ZeroRotator;
	}

	bIsRotationMode = bActive;
}