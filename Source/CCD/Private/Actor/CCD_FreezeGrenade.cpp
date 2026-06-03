// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/CCD_FreezeGrenade.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "NiagaraComponentPoolMethodEnum.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
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
		Detonate(OtherActor);
	}
}

void ACCD_FreezeGrenade::Detonate(AActor* TargetActor)
{
    if (!HasAuthority() || !TargetActor) return;
    
    APawn* TargetPawn = Cast<APawn>(TargetActor);
    if (TargetPawn)
    {
        if (AAIController* AIC = Cast<AAIController>(TargetPawn->GetController()))
        {
            if (UBrainComponent* BrainComp = AIC->FindComponentByClass<UBrainComponent>())
            {
                BrainComp->StopLogic(TEXT("Freeze Grenade Direct Hit"));
                AIC->StopMovement();
                
                USkeletalMeshComponent* TargetMesh = TargetPawn->FindComponentByClass<USkeletalMeshComponent>();
                if (TargetMesh)
                {
                    TargetMesh->bNoSkeletonUpdate = true;
                    TargetMesh->SetComponentTickEnabled(false);
                }
            	
                TWeakObjectPtr<UBrainComponent> WeakBrainComp = BrainComp;
                TWeakObjectPtr<AAIController> WeakAIC = AIC;
                TWeakObjectPtr<USkeletalMeshComponent> WeakTargetMesh = TargetMesh;

                FTimerHandle UnfreezeTimerHandle;
                GetWorldTimerManager().SetTimer(UnfreezeTimerHandle, FTimerDelegate::CreateLambda([WeakBrainComp, WeakAIC, WeakTargetMesh]()
                {
                    if (WeakBrainComp.IsValid() && WeakAIC.IsValid())
                    {
                        WeakBrainComp->RestartLogic();
                        
                        if (WeakTargetMesh.IsValid())
                        {
                            WeakTargetMesh->bNoSkeletonUpdate = false;
                            WeakTargetMesh->SetComponentTickEnabled(true);
                        }
                        
                        UE_LOG(LogTemp, Warning, TEXT("[Grenade] Direct-hit AI and Animation Restarted successfully after 30s."));
                    }
                    else
                    {
                        UE_LOG(LogTemp, Log, TEXT("[Grenade] Target AI or Controller was destroyed before unfreeze timer ended."));
                    }
                }), 30.0f, false);
                
                UE_LOG(LogTemp, Warning, TEXT("[Grenade] Direct Hit! Stopped Brain and Anim for: %s"), *TargetActor->GetName());
            }
        }
    }
    
    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
    }

    if (ExplosionVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionVFX, GetActorLocation());
    }
    
    Destroy();
}