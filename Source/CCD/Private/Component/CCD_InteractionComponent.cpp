
#include "Component/CCD_InteractionComponent.h"
#include "CCDCharacter.h"
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

void UCCD_InteractionComponent::PhysicsHandleUpdate(float DeltaTime)
{
	if (!PhysicsHandle || !GrabbedComponent) return;
    
	// 진짜 목표 지점
	float TargetDistance = 200.f; // 이 수치를 조절하여 물체와의 거리 변경 가능
	FVector RealTargetLocation = OwnerCharacter->GetFirstPersonCamera()->GetComponentLocation() + (OwnerCharacter->GetFirstPersonCamera()->GetForwardVector() * TargetDistance);
	FRotator RealTargetRotation = OwnerCharacter->GetFirstPersonCamera()->GetComponentRotation();
	
	// 현재 핸들 위치와 회전 가져오기
	FVector CurrentLocation {};
	FRotator CurrentRotation {};
	PhysicsHandle->GetTargetLocationAndRotation(CurrentLocation, CurrentRotation);
	
	// 보간 계산
	float FollowSpeed = 10.0f; // 이 수치를 조절하여 따라가는 속도 변경 가능
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, RealTargetLocation, DeltaTime, FollowSpeed);
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, RealTargetRotation, DeltaTime, FollowSpeed);
	
	PhysicsHandle->SetTargetLocationAndRotation(NewLocation, NewRotation);
}

void UCCD_InteractionComponent::PerformInteract()
{
	Server_PerformInteract();
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
			UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(HitActor->GetRootComponent());
			if (RootPrim && RootPrim->IsSimulatingPhysics())
			{
				Multicast_GrabObject(RootPrim, HitResult.ImpactPoint);
			}
			IInteractInterface::Execute_Interact(BurnComp, OwnerCharacter);
		}
		else if (HitActor->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
		{
			IInteractInterface::Execute_Interact(HitActor, OwnerCharacter);
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

	GrabbedComponent = ComponentToGrab;
	
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
		ComponentToGrab->GetComponentRotation()
	);
}

void UCCD_InteractionComponent::Multicast_ReleaseObject_Implementation()
{
	ReleaseObject_Impl();
}
void UCCD_InteractionComponent::ReleaseObject_Impl()
{
	if (!OwnerCharacter) return;
	if (!PhysicsHandle || !GrabbedComponent) return;
	
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
}