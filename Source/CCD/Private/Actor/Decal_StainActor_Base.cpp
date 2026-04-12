
#include "Actor/Decal_StainActor_Base.h"

#include "Component/ProgressComponent.h"
#include "Component/WashableComponent.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/CCDCharacter.h"

ADecal_StainActor_Base::ADecal_StainActor_Base()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 네트워크 복제 설정
	bReplicates = true;
	bNetLoadOnClient = true;
	SetReplicatingMovement(true);
	
	// 컴포넌트 생성 및 포함
	ProgressComp = CreateDefaultSubobject<UProgressComponent>(TEXT("ProgressComp"));
	ProgressComp->SetIsReplicated(true);
	WashableComp = CreateDefaultSubobject<UWashableComponent>(TEXT("WashableComp"));
	WashableComp->SetIsReplicated(true);
	
	// 부모 데칼 컴포넌트의 복제 설정
	GetDecal()->SetIsReplicated(true);
	
	// 트리거 설정
	StepTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("StepTrigger"));
	StepTrigger->SetupAttachment(RootComponent);
	StepTrigger->SetCollisionProfileName(TEXT("Trigger"));
	StepTrigger->SetBoxExtent(FVector(10.f, 100.f, 100.f)); 
    
	StepTrigger->OnComponentBeginOverlap.AddDynamic(this, &ADecal_StainActor_Base::OnStepTriggerBeginOverlap);
	
	AllowedSurfaceTags = { FName("Floor"), FName("Wall") };
}

void ADecal_StainActor_Base::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		ValidateSurface(); // 스폰 시점에 해당 위치가 유효한지 검사
	}
	
	SetActorTickEnabled(false); // 초기에는 Tick 비활성화 -> 양동이에서 스폰 시 조건에 맞춰 활성화
	
	// 부모 클래스인 ADecalActor가 가진 Decal 컴포넌트를 가져옴
	UDecalComponent* DecalComp = GetDecal();
	if (DecalComp && WashableComp->GetWashableType() != ECCD_WashableType::EWT_Water)
	{
		// 0번 슬롯의 머티리얼로 DMI 생성
		UMaterialInterface* BaseMat = DecalComp->GetDecalMaterial();
		if (BaseMat)
		{
			DecalDMI = DecalComp->CreateDynamicMaterialInstance();
		}
	}
	UpdateDecalOpacity(WashableComp->GetWashHealthRatio());
}

void ADecal_StainActor_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UE_LOG(LogTemp, Warning, TEXT("%s - Tick : %f"), *GetName(), DeltaTime); // Tick 최적화 검증용
	
	if (WashableComp->GetWashableType() == ECCD_WashableType::EWT_Water && ProgressComp->ProgressValue <= 0.0f)
	{
		// 시간을 누적하여 직접 투명도 계산 (LifeSpan 의존성 제거)
		FadeTimeAccumulator += DeltaTime;
        
		// 서서히 투명해짐
		const float FadeDuration = 3.0f;
		const float Opacity = FMath::Clamp(1.0f - (FadeTimeAccumulator / FadeDuration), 0.0f, 1.0f);
        
		UpdateDecalOpacity(Opacity);
		
		if (Opacity <= 0.0f)
		{
			Destroy();
		}
	}
}

void ADecal_StainActor_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);	
	DOREPLIFETIME(ADecal_StainActor_Base, DecalColor);
}

void ADecal_StainActor_Base::UpdateDecalOpacity(float NewRatio) const
{
	if (DecalDMI)
	{
		// 머티리얼의 Scalar Parameter 업데이트
		DecalDMI->SetScalarParameterValue(TEXT("Opacity Intensity"), NewRatio);
	}
}

void ADecal_StainActor_Base::UseWaterDecalMaterial()
{
	UDecalComponent* DecalComp = GetDecal();
	DecalComp->SetDecalMaterial(WaterDecalMaterial);
	
	UMaterialInterface* BaseMat = DecalComp->GetDecalMaterial();
	if (BaseMat)
	{
		DecalDMI = DecalComp->CreateDynamicMaterialInstance();
		
		if (DecalDMI)
		{
			DecalDMI->SetVectorParameterValue(TEXT("BaseColor Tint"), DecalColor);
		}
	}
	
	UpdateDecalOpacity(WashableComp->GetWashHealthRatio());
}

void ADecal_StainActor_Base::OnRep_DecalColor()
{
	if (DecalDMI)
	{
		DecalDMI->SetVectorParameterValue(TEXT("BaseColor Tint"), DecalColor);
	}
}

void ADecal_StainActor_Base::ValidateSurface()
{
	if (!HasAuthority()) return;
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // 기본적으로 자기 자신 무시
	
	const FVector OriginalUpVector = GetActorUpVector(); // 현재 액터의 회전 정보 보관
	
	const int MaxAttempts = 1; // 최대 시도 횟수
	for (int i = 0; i < MaxAttempts; ++i)
	{
		// 스폰 시점에 데칼의 투영 방향으로 짧은 트레이스 수행
		FHitResult Hit;
		FVector Start = GetActorLocation() - (GetActorForwardVector() * 20.0f);
		FVector End = Start + (GetActorForwardVector() * 100.0f);

		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_DecalSurface, Params))
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor)
			{
				// --- 태그 검사 ---
				bool bHasValidTag = false;
			
				// 에디터에서 설정한 허용 태그 리스트를 순회하며 검사
				for (const FName& Tag : AllowedSurfaceTags)
				{
					if (HitActor->ActorHasTag(Tag))
					{
						bHasValidTag = true;
						break;
					}
				}

				// 태그 검사에 성공 -> 해당 표면으로 위치 및 회전 보정 후 종료
				if (bHasValidTag)
				{
					FVector NewLocation = Hit.ImpactPoint + (Hit.ImpactNormal * 0.5f);
					
					FVector ForwardVector = -Hit.ImpactNormal;
					FRotator NewRotation = FRotationMatrix::MakeFromXZ(ForwardVector, OriginalUpVector).Rotator();
					
					// FRotator NewRotation = Hit.ImpactNormal.Rotation();
					// NewRotation.Pitch -= 180.0f;
					
					SetActorLocationAndRotation(NewLocation, NewRotation);
					return;
				}
				
				Params.AddIgnoredActor(HitActor);
				// UE_LOG(LogTemp, Warning, TEXT("%s - LineTrace hit actor %s, but no tags. Attempt %d/%d"), *GetName(), *HitActor->GetName(), i + 1, MaxAttempts);
			}
		}
		else
		{
			// UE_LOG(LogTemp, Warning, TEXT("%s - LineTrace No Hit"), *GetName());
			break; 
		}
	}
	
	// 3회 시도 후에도 유효한 표면이 감지되지 않으면 데칼 제거
	UE_LOG(LogTemp, Warning, TEXT("%s - Invalid surface. Destroying decal."), *GetName());
	Destroy();
}

void ADecal_StainActor_Base::OnStepTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return; // 서버에서만 처리
	
	if (!bCanStainFeet) return;

	// 물(Water) 데칼인 경우에는 피가 묻지 않도록 처리 (선택 사항)
	if (WashableComp->GetWashableType() == ECCD_WashableType::EWT_Water) return;

	if (ACCDCharacter* Character = Cast<ACCDCharacter>(OtherActor))
	{
		// 캐릭터에게 피가 묻었음을 알림
		Character->AddBloodToFeet(6);
	}
}