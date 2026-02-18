
#include "Actor/CCD_EMopActor.h"

#include <Camera/CameraComponent.h>
#include <Net/UnrealNetwork.h>

#include "CCDCharacter.h"
#include "Actor/WaterBucketActor.h"
#include "Component/WashableComponent.h"

ACCD_EMopActor::ACCD_EMopActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ACCD_EMopActor::ExecuteAction()
{
	// 서버에서만 세척 판정 수행
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteAction :: MOP"));
		
		if (OwnerCharacter) 
			OwnerCharacter->Server_PlayActionOfMop_Implementation();
		PerformMopTrace();
	}
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

		// 1. 물양동이 처리
		if (AWaterBucketActor* Bucket = Cast<AWaterBucketActor>(HitActor))
		{
			if (Bucket->WashMop(MopPollution_Blood, MopPollution_Excrement))
			{
				UpdateMopMaterial();
			}
			return;
		}

		// 2. 세척 가능 컴포넌트 처리 (데칼 등)
		if (UWashableComponent* WashComp = HitActor->FindComponentByClass<UWashableComponent>())
		{
			if (MopPollution_Blood + MopPollution_Excrement >= 1.0f) return;

			WashComp->TakeWashDamage(25.f);
			if (WashComp->GetWashableType() == ECCD_WashableType::EWT_Blood) MopPollution_Blood += 0.2f;
			else MopPollution_Excrement += 0.2f;
			
			UpdateMopMaterial();
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
		
		DynamicMopMaterial->SetVectorParameterValue(TEXT("BaseColor"), FinalColor);
	}
}

void ACCD_EMopActor::OnRep_Pollution() { UpdateMopMaterial(); }
