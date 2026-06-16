// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/CCD_FreezeGrenade.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "NiagaraComponentPoolMethodEnum.h"
#include "NiagaraFunctionLibrary.h"
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
    
	// 1. 구체 형태의 물리 범위 검사(Sphere Overlap) 세팅
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(FreezeRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (GetOwner()) QueryParams.AddIgnoredActor(GetOwner()); // 던진 플레이어 무시
	GetWorld()->OverlapMultiByChannel(OverlapResults, ExplodeLocation, FQuat::Identity, ECC_Pawn, SphereShape, QueryParams);
	
    // 2. 감지된 오버랩 액터들을 순회하며 AI 검출 및 동결 적용
    for (const FOverlapResult& Result : OverlapResults)
    {
        AActor* OverlappedActor = Result.GetActor();
        if (!OverlappedActor || OverlappedActor->IsA(ACCDCharacter::StaticClass())) continue; // 다른 유저 플레이어면 패스
        
        APawn* TargetPawn = Cast<APawn>(OverlappedActor);
        if (TargetPawn)
        {
            // 오버랩된 생명체가 플레이어가 아닌 'AI 컨트롤러'를 지니고 있는지 판별
            if (AAIController* AIC = Cast<AAIController>(TargetPawn->GetController()))
            {
                if (UBrainComponent* BrainComp = AIC->FindComponentByClass<UBrainComponent>())
                {
                    // AI 사고 정지 및 즉시 정지
                    BrainComp->StopLogic(TEXT("Freeze Grenade Radial Hit"));
                    AIC->StopMovement();
                    
                    // 애니메이션 일시정지 처리
                    USkeletalMeshComponent* TargetMesh = TargetPawn->FindComponentByClass<USkeletalMeshComponent>();
                    if (TargetMesh)
                    {
                        TargetMesh->bNoSkeletonUpdate = true;
                        TargetMesh->SetComponentTickEnabled(false);
                    }
                	
                    // 개별 람다 캡처용 약포인터 생성
                    TWeakObjectPtr<UBrainComponent> WeakBrainComp = BrainComp;
                    TWeakObjectPtr<AAIController> WeakAIC = AIC;
                    TWeakObjectPtr<USkeletalMeshComponent> WeakTargetMesh = TargetMesh;

                    // 각 대상마다 독립적인 30초 타이머 작동
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
                            
                            UE_LOG(LogTemp, Warning, TEXT("[Grenade] Radial-hit AI and Animation Restarted successfully after 30s."));
                        }
                    }), 30.0f, false);
                    
                    UE_LOG(LogTemp, Warning, TEXT("[Grenade] Radial Hit! Stopped Brain and Anim for: %s"), *OverlappedActor->GetName());
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