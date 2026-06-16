// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/CCD_FreezeGrenade.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "NiagaraComponentPoolMethodEnum.h"
#include "NiagaraFunctionLibrary.h"
#include "AI/CCD_173.h"
#include "AI/CCD_939.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CCDCharacter.h"

ACCD_FreezeGrenade::ACCD_FreezeGrenade()
{
	bReplicates = true;
	AActor::SetReplicateMovement(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.3f;
	ProjectileMovement->bAutoActivate = false;
}

void ACCD_FreezeGrenade::BeginPlay()
{
	Super::BeginPlay();
	
	if (MeshComp)
	{
		ProjectileMovement->UpdatedComponent = MeshComp;
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));
		
		MeshComp->OnComponentHit.AddDynamic(this, &ACCD_FreezeGrenade::OnGrenadeHit);
	}
}

void ACCD_FreezeGrenade::Launch(FVector LaunchDirection)
{
	if (bIsLaunched || !MeshComp || !ProjectileMovement) return;
	bIsLaunched = true;
	
	MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
	MeshComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	
	MeshComp->SetCollisionProfileName(TEXT("Projectile"));
	MeshComp->SetSimulatePhysics(true);
	
	ProjectileMovement->InitialSpeed = 1800.0f;
	ProjectileMovement->MaxSpeed = 1800.0f;
	
	ProjectileMovement->SetComponentTickEnabled(true);
	ProjectileMovement->SetActive(true);
	ProjectileMovement->Velocity = LaunchDirection * ProjectileMovement->InitialSpeed;
	
	MeshComp->SetPhysicsLinearVelocity(ProjectileMovement->Velocity);
}

void ACCD_FreezeGrenade::OnGrenadeHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bIsLaunched || !OtherActor) return;
	
	if (OtherActor == GetOwner() || OtherActor->IsA(ACCDCharacter::StaticClass()))
	{
		return;
	}
	
	if (HasAuthority())
	{
		Detonate();
	}
}

void ACCD_FreezeGrenade::Detonate()
{
    if (!HasAuthority()) return;
    
    FVector ExplodeLocation = GetActorLocation();
	
    TArray<FOverlapResult> OverlapResults;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(FreezeRadius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    if (GetOwner()) QueryParams.AddIgnoredActor(GetOwner());
    GetWorld()->OverlapMultiByChannel(OverlapResults, ExplodeLocation, FQuat::Identity, ECC_Pawn, SphereShape, QueryParams);
	
    for (const FOverlapResult& Result : OverlapResults)
    {
        AActor* OverlappedActor = Result.GetActor();
        if (!OverlappedActor || OverlappedActor->IsA(ACCDCharacter::StaticClass())) continue; // 다른 유저 플레이어면 패스
        
        APawn* TargetPawn = Cast<APawn>(OverlappedActor);
        if (TargetPawn)
        {
            if (AAIController* AIC = Cast<AAIController>(TargetPawn->GetController()))
            {
                if (UBrainComponent* BrainComp = AIC->FindComponentByClass<UBrainComponent>())
                {
                    BrainComp->StopLogic(TEXT("Freeze Grenade Radial Hit"));
                    AIC->StopMovement();
                    USkeletalMeshComponent* TargetMesh = TargetPawn->FindComponentByClass<USkeletalMeshComponent>();
                    if (TargetMesh)
                    {
                        TargetMesh->bNoSkeletonUpdate = true;
                        TargetMesh->SetComponentTickEnabled(false);
                    }
                	
                    ACCD_096* SCP096 = Cast<ACCD_096>(TargetPawn);
                    ACCD_173* SCP173 = Cast<ACCD_173>(TargetPawn);
                    ACCD_939* SCP939 = Cast<ACCD_939>(TargetPawn);

                    if (SCP096)
                    {
                        SCP096->Multicast_SetFreezeVisual(true);
                    }
                    if (SCP939)
                    {
                        SCP939->Multicast_SetFreezeVisual(true);
                    }
                    if (SCP173)
                    {
                        SCP173->Multicast_SetFreezeVisual(true);
                        SCP173->StopMoveSound();
                    }
                	
                    TWeakObjectPtr<UBrainComponent> WeakBrainComp = BrainComp;
                    TWeakObjectPtr<AAIController> WeakAIC = AIC;
                    TWeakObjectPtr<USkeletalMeshComponent> WeakTargetMesh = TargetMesh;
                    
                    TWeakObjectPtr<ACCD_096> Weak096 = SCP096;
                    TWeakObjectPtr<ACCD_173> Weak173 = SCP173;
                    TWeakObjectPtr<ACCD_939> Weak939 = SCP939;
                	
                    FTimerHandle UnfreezeTimerHandle;
                    GetWorldTimerManager().SetTimer(UnfreezeTimerHandle, FTimerDelegate::CreateLambda([WeakBrainComp, WeakAIC, WeakTargetMesh, Weak096, Weak173, Weak939]()
                    {
                        if (WeakBrainComp.IsValid() && WeakAIC.IsValid())
                        {
                            WeakBrainComp->RestartLogic();
                            
                            if (WeakTargetMesh.IsValid())
                            {
                                WeakTargetMesh->bNoSkeletonUpdate = false;
                                WeakTargetMesh->SetComponentTickEnabled(true);
                            }

                            if (Weak096.IsValid()) Weak096->Multicast_SetFreezeVisual(false);
                            if (Weak939.IsValid()) Weak939->Multicast_SetFreezeVisual(false);
                            if (Weak173.IsValid()) Weak173->Multicast_SetFreezeVisual(false);
                            
                            UE_LOG(LogTemp, Warning, TEXT("[Grenade] Radial-hit SCP AI, Anim, and Ice Visual Restored successfully."));
                        }
                    }), 30.0f, false);
                    
                    UE_LOG(LogTemp, Warning, TEXT("[Grenade] Radial Hit! Stopped Brain, Anim, and Frozen Visual for: %s"), *OverlappedActor->GetName());
                }
            }
        }
    }
    
    Multicast_PlayExplosionEffects(GetActorLocation());
    
    Destroy();
}

void ACCD_FreezeGrenade::Multicast_PlayExplosionEffects_Implementation(FVector Location)
{
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, Location);
	}

	if (ExplosionVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionVFX, Location);
	}
}