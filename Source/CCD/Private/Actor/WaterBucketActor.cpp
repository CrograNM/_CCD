
#include "Actor/WaterBucketActor.h"

#include "NiagaraFunctionLibrary.h"
#include "Actor/Decal_StainActor_Base.h"
#include "Component/ProgressComponent.h"
#include "Component/WashableComponent.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


AWaterBucketActor::AWaterBucketActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.2f;
	
	// 물 양동이 액터는 초기 진행도를 0으로 설정
	ProgressComp->ProgressValue = 0.0f;
	
	WaterMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMeshComp"));
	WaterMeshComp->SetupAttachment(RootComponent); 
	WaterMeshComp->SetIsReplicated(true);
}

void AWaterBucketActor::BeginPlay()
{
	Super::BeginPlay();
	
	// 머티리얼 인스턴스 생성 (블루프린트에서 StaticMesh에 할당된 재질 인덱스 0번 가정)
	if (WaterMeshComp)
	{
		WaterMaterial = WaterMeshComp->CreateAndSetMaterialInstanceDynamic(0);
		UpdateWaterColor();
	}
}

void AWaterBucketActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// UE_LOG(LogTemp, Warning, TEXT("%s - Tick : %f"), *GetName(), DeltaTime); // Tick 최적화 검증용
	
	// 일정 이상의 각도에 도달하면 물이 쏟아지는 효과
	if (WaterMaterial && HasAuthority())
	{
		FRotator Rotation = GetActorRotation();
		if (FMath::Abs(Rotation.Pitch) > 60.0f || FMath::Abs(Rotation.Roll) > 60.0f)
			SpillWater();
	}
	if (bIsWaterSpilled) SetActorTickEnabled(false);
}

void AWaterBucketActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AWaterBucketActor, Pollution_Blood);
	DOREPLIFETIME(AWaterBucketActor, Pollution_Excrement);
}

bool AWaterBucketActor::WashMop(float& InBloodAmount, float& InExcrementAmount)
{
	if (bIsWaterSpilled) return false;
	if (HasAuthority())
	{
		Multicast_PlayWashSoundEffect();
	} else return false;
	if (Pollution_Blood + Pollution_Excrement >= 1.0f)
	{
		return false;
	}

	// 물양동이에 오염물질 전이
	float MaxPollutionValue = 0.2f; // 고정최대값으로 오염됨 ( 0.2x5=1 -> 5회 사용 가능 )
	Pollution_Blood += FMath::Clamp(InBloodAmount, 0.0f, MaxPollutionValue);
	Pollution_Excrement += FMath::Clamp(InExcrementAmount, 0.0f, MaxPollutionValue);
	Pollution_Blood = FMath::Clamp(Pollution_Blood, 0.0f, 1.0f);
	Pollution_Excrement = FMath::Clamp(Pollution_Excrement, 0.0f, 1.0f);

	// 대걸레는 깨끗해짐 (참조로 받아온 변수 수정)
	float CleanValue = 1.0f; // 고정값으로 깨끗해짐 (한번에 완전 세척)
	InBloodAmount -= CleanValue;
	InExcrementAmount -= CleanValue;
	InBloodAmount = FMath::Clamp(InBloodAmount, 0.0f, 1.0f);
	InExcrementAmount = FMath::Clamp(InExcrementAmount, 0.0f, 1.0f);
	
	// 양동이의 진행도도 업데이트 (오염도 합계 * 10) = 최대 10점 (데칼 기본 점수)
	ProgressComp->UpdateProgressValue((Pollution_Blood + Pollution_Excrement) * 10.0f);
	
	// 서버에서도 시각적 업데이트
	OnRep_Pollution();

	return true;
}

void AWaterBucketActor::OnRep_IsWaterSpilled()
{
	if (SpillSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpillSound, 
			WaterMeshComp->GetComponentLocation()); 
	}
	if (SpillEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SpillEffect, 
			WaterMeshComp->GetComponentLocation());
	}
}

void AWaterBucketActor::OnRep_Pollution()
{
	UpdateWaterColor();
}

void AWaterBucketActor::UpdateWaterColor()
{
	if (WaterMaterial)
	{
		// 머티리얼 파라미터 제어 (예: BloodAmount, PoopAmount)
		WaterMaterial->SetScalarParameterValue(TEXT("BloodIntensity"), Pollution_Blood);
		WaterMaterial->SetScalarParameterValue(TEXT("ExcrementIntensity"), Pollution_Excrement);
        
		// 혹은 두 색상을 섞어서 BaseColor 변경
		FLinearColor CleanColor = FLinearColor(0.228f, 0.343f, 0.405f, 1.0f); // 깨끗한 물 색상
		FLinearColor BloodColor = FLinearColor(0.69f, 0.13f, 0.13f, 1.0f); // 핏빛
		FLinearColor PoopColor = FLinearColor(0.0f, 0.5f, 0.0f, 1.0f); // 배설물
	
		WaterColor = CleanColor;
		WaterColor = FMath::Lerp(WaterColor, BloodColor, Pollution_Blood);
		WaterColor = FMath::Lerp(WaterColor, PoopColor, Pollution_Excrement);
        
		WaterMaterial->SetVectorParameterValue(TEXT("WaterColor"), WaterColor);
	}
}

void AWaterBucketActor::SpillWater()
{
	if (!HasAuthority() || !WaterMeshComp->IsVisible()) return;
	
	// 바닥 추적 (Line Trace)
	FHitResult HitResult;
	FVector Start = GetActorLocation();
	FVector End = Start + FVector(0, 0, -500.0f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		// 바닥 평면에 맞춘 회전값 (바닥 노멀 기준)
		FRotator SpawnRot = HitResult.ImpactNormal.Rotation();
		SpawnRot.Pitch -= 90.0f; // 데칼은 기본적으로 X축 방향으로 쏘므로 아래를 향하게 조정
		
		if (ADecal_StainActor_Base* SpawnedDecal = GetWorld()->SpawnActor<ADecal_StainActor_Base>(
			DecalStainActorClass, HitResult.Location, SpawnRot, SpawnParams))
		{
			if (UWashableComponent* DecalWashComp = SpawnedDecal->FindComponentByClass<UWashableComponent>())
			{
				DecalWashComp->SetWashableType(ECCD_WashableType::EWT_Water);
			}

			SpawnedDecal->DecalColor = WaterColor;
			if (SpawnedDecal->DecalDMI)
			{
				SpawnedDecal->DecalDMI->SetVectorParameterValue(TEXT("BaseColor Tint"), WaterColor);
			}
			
			// 진행도(Progress) 전이 로직
			if (UProgressComponent* DecalProg = SpawnedDecal->FindComponentByClass<UProgressComponent>())
			{
				// 양동이가 가지고 있던 점수를 데칼에게 그대로 전달
				DecalProg->UpdateProgressValue(ProgressComp->ProgressValue);
				if (ProgressComp->ProgressValue <= 0.0f)
				{
					SpawnedDecal->SetActorTickEnabled(true); // 진행도가 0이면 데칼이 서서히 사라지는 효과 활성화
					SpawnedDecal->SetActorTickInterval(0.1f); // 초당 10회 제한
				}
			}
			ProgressComp->UpdateProgressValue(0.0f); // 양동이의 진행도는 0으로 리셋
		}
	}
		
	UE_LOG(LogTemp, Warning, TEXT("Spill Water!"));
	WaterMeshComp->SetVisibility(false);
	WaterMaterial = nullptr;
	bIsWaterSpilled = true;
	OnRep_IsWaterSpilled();
}

void AWaterBucketActor::Multicast_PlayWashSoundEffect_Implementation()
{
	if (WashSplashSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WashSplashSound, WaterMeshComp->GetComponentLocation());
	}
	if (WashSplashEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WashSplashEffect, WaterMeshComp->GetComponentLocation());
	}
}
