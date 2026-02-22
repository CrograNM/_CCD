
#include "Actor/CCD_EMopActor.h"

#include <Camera/CameraComponent.h>
#include <Net/UnrealNetwork.h>

#include "CCDCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "Actor/WaterBucketActor.h"
#include "Component/WashableComponent.h"
#include "Kismet/GameplayStatics.h"

ACCD_EMopActor::ACCD_EMopActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ACCD_EMopActor::ExecuteAction()
{
	if (!HasAuthority()) return;
	if (OwnerCharacter) OwnerCharacter->Server_PlayActionOfMop();
	
	Multicast_PlayMopSwingSound();
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
		if (UWashableComponent* WashComp = HitActor->FindComponentByClass<UWashableComponent>())
		{
			if (MopPollution_Blood + MopPollution_Excrement >= 1.0f) return;

			WashComp->TakeWashDamage(50.f);
			if (WashComp->GetWashableType() == ECCD_WashableType::EWT_Blood) MopPollution_Blood += 0.2f;
			else MopPollution_Excrement += 0.2f;
			
			UpdateMopMaterial();
			
			// 세척 효과 재생 (모든 클라이언트에서)
			Multicast_PlayWashEffect(HitResult.ImpactPoint);
		}
	}
}

void ACCD_EMopActor::UpdateMopMaterial()
{
	if (DynamicMopMaterial)
	{
		// 혹은 두 색상을 섞어서 BaseColor 변경
		FLinearColor CleanColor = FLinearColor(0.228f, 0.343f, 0.405f, 1.0f); // 깨끗한 물 색상
		FLinearColor BloodColor = FLinearColor(0.69f, 0.13f, 0.13f, 1.0f); // 핏빛
		FLinearColor PoopColor = FLinearColor(0.0f, 0.5f, 0.0f, 1.0f); // 배설물

		FLinearColor FinalColor = CleanColor;
		FinalColor = FMath::Lerp(FinalColor, BloodColor, MopPollution_Blood);
		FinalColor = FMath::Lerp(FinalColor, PoopColor, MopPollution_Excrement);
		
		// 이후 대걸레 자체의 머티리얼 파라미터를 제어해야함 (현재는 임시 머티리얼로 M_BucketWater 사용 중)
		DynamicMopMaterial->SetVectorParameterValue(TEXT("BaseColor"), FinalColor);
	}
}

void ACCD_EMopActor::OnRep_Pollution() { UpdateMopMaterial(); }

void ACCD_EMopActor::Multicast_PlayWashEffect_Implementation(const FVector_NetQuantize& ImpactPoint)
{
	if (MopWashSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MopWashSound, ImpactPoint);
	}
	if (MopWashEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MopWashEffect, ImpactPoint);
	}
}

void ACCD_EMopActor::Multicast_PlayMopSwingSound_Implementation()
{
	if (MopSwingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MopSwingSound, GetActorLocation());
	}
}
