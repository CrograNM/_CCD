#include "Actor/CCD_EMopActor.h"

#include <Camera/CameraComponent.h>
#include <Net/UnrealNetwork.h>

#include "Player/CCDCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "Actor/WaterBucketActor.h"
#include "Component/WashableComponent.h"
#include "Kismet/GameplayStatics.h"

ACCD_EMopActor::ACCD_EMopActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	// 메쉬 생성 및 루트 설정
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// 기본적으로 물리 연산, 콜리전 무시
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));
}

void ACCD_EMopActor::ExecuteAction()
{
	if (!HasAuthority()) return;
	if (!OwnerCharacter) return;
	if (OwnerCharacter->GetIsActionInProgress()) return;
	
	OwnerCharacter->Server_PlayActionOfMop();
}

void ACCD_EMopActor::BeginPlay()
{
	Super::BeginPlay();
	if (MeshComp)
	{
		DynamicMopMaterial = MeshComp->CreateDynamicMaterialInstance(0);
		UpdateMopMaterial();
	}
}

void ACCD_EMopActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACCD_EMopActor, MopPollution_Blood);
	DOREPLIFETIME(ACCD_EMopActor, MopPollution_Excrement);
}

void ACCD_EMopActor::PerformMopTrace()
{
	if (!HasAuthority()) return;
	if (!OwnerCharacter) return;

	// 카메라 위치와 방향 가져오기
	FVector Start = OwnerCharacter->GetFirstPersonCamera()->GetComponentLocation();
	FVector End = Start + (OwnerCharacter->GetFirstPersonCamera()->GetForwardVector() * 300.f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActor) return;
		
		// 세척 효과 재생 (일단 맞은 액터가 있으면)
		Multicast_PlayWashEffect(HitResult.ImpactPoint, HitResult.ImpactNormal);

		// 물양동이 처리
		if (AWaterBucketActor* Bucket = Cast<AWaterBucketActor>(HitActor))
		{
			if (Bucket->WashMop(MopPollution_Blood, MopPollution_Excrement))
			{
				UpdateMopMaterial();
			}
			return;
		}

		// 데칼 세척 처리
		if (MopPollution_Blood + MopPollution_Excrement >= 1.0f) return;
		if (UWashableComponent* WashComp = HitActor->FindComponentByClass<UWashableComponent>())
		{
			WashComp->TakeWashDamage(50.f);
			
			const float PollutionAdded = 1.0f / MaxUseCount;
			if (WashComp->GetWashableType() == ECCD_WashableType::EWT_Blood) 
				MopPollution_Blood += PollutionAdded;
			else if (WashComp->GetWashableType() == ECCD_WashableType::EWT_Excrement) 
				MopPollution_Excrement += PollutionAdded;
			
			UpdateMopMaterial();
		}
		
	}
	else {
		// 대걸레 허공 휘두르기 사운드 재생
		Multicast_PlayMopSwingSound();
	}
}

void ACCD_EMopActor::UpdateMopMaterial()
{
	if (!DynamicMopMaterial) return;
	float TotalPollution = MopPollution_Blood + MopPollution_Excrement;
	if (TotalPollution <= 0.001f)
	{
		DynamicMopMaterial->SetScalarParameterValue(TEXT("PollutionRate"), 0.0f); 
		return;
	}
	// 오염 정도를 0.5~1.0 범위로 매핑하여 머티리얼에 전달
	TotalPollution = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 1.f), FVector2D(0.5f, 1.f), TotalPollution);
	DynamicMopMaterial->SetScalarParameterValue(TEXT("PollutionRate"), TotalPollution); 
	UE_LOG(LogTemp, Warning, TEXT("TotalPollution: %f"), TotalPollution);
	
	FLinearColor BloodColor = FLinearColor(0.15f, 0.05, 0.05f, 1.00f); // 핏빛
	FLinearColor PoopColor = FLinearColor(0.32f, 0.35f, 0.06f, 1.00f); // 배설물
	
	FLinearColor FinalColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.00f);
	FinalColor = FMath::Lerp(FinalColor, BloodColor, MopPollution_Blood);
	FinalColor = FMath::Lerp(FinalColor, PoopColor, MopPollution_Excrement);
	
	DynamicMopMaterial->SetVectorParameterValue(TEXT("PollutionColor"), FinalColor);
}

void ACCD_EMopActor::OnRep_Pollution() { UpdateMopMaterial(); }

void ACCD_EMopActor::Multicast_PlayWashEffect_Implementation(const FVector_NetQuantize& ImpactPoint, const FVector_NetQuantizeNormal& ImpactNormal)
{
	if (MopWashSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MopWashSound, ImpactPoint);
	}
	if (MopWashEffect)
	{
		FRotator SpawnRot = ImpactNormal.Rotation();
		SpawnRot.Pitch -= 90.0f;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MopWashEffect, ImpactPoint, SpawnRot);
	}
}

void ACCD_EMopActor::Multicast_PlayMopSwingSound_Implementation()
{
	if (MopSwingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MopSwingSound, GetActorLocation());
	}
}
