
#include "Actor/IncineratorActor.h"
#include "Components/BoxComponent.h"
#include "Component/BurnableComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ActorSequenceComponent.h"
#include "ActorSequencePlayer.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Hearing.h"

AIncineratorActor::AIncineratorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	
	// 메쉬 컴포넌트 설정
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
	RootComponent = MainMesh;
	
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(RootComponent); 
	
	// 소각 영역 설정
	BurnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("BurnArea"));
	BurnArea->SetupAttachment(RootComponent);
	
	// 서버에서만 대미지 판정을 하도록 설정
	BurnArea->SetCollisionProfileName(TEXT("Trigger"));
	
	// 오디오 컴포넌트 생성 및 설정
	BurningAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("BurningAudioComp"));
	BurningAudioComp->SetupAttachment(RootComponent);
	BurningAudioComp->bAutoActivate = false; // 처음부터 켜지지 않게 설정
}

void AIncineratorActor::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorTickEnabled(false);
	
	if (BurningAudioComp && BurningSound)
	{
		BurningAudioComp->SetSound(BurningSound);
	}
	
	if (HasAuthority())
	{
		BurnArea->OnComponentBeginOverlap.AddDynamic(this, &AIncineratorActor::OnBurnAreaBeginOverlap);
		BurnArea->OnComponentEndOverlap.AddDynamic(this, &AIncineratorActor::OnBurnAreaEndOverlap);
	}
}

void AIncineratorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UE_LOG(LogTemp, Warning, TEXT("%s - Tick : %f"), *GetName(), DeltaTime); // Tick 최적화 검증용
	
	ApplyBurnDamage(DeltaTime);
}

void AIncineratorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AIncineratorActor, bIsDoorOpen);
}

void AIncineratorActor::OnBurnAreaBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (UBurnableComponent* BurnComp = OtherActor->FindComponentByClass<UBurnableComponent>())
		{
			OverlappingBurnables.AddUnique(BurnComp);
			UE_LOG(LogTemp, Warning, TEXT("Overlapped Incinerator : %s"), *OtherActor->GetName());
		}
	}
}

void AIncineratorActor::OnBurnAreaEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		if (UBurnableComponent* BurnComp = OtherActor->FindComponentByClass<UBurnableComponent>())
		{
			OverlappingBurnables.Remove(BurnComp);
		}
	}
}

void AIncineratorActor::ApplyBurnDamage(float DeltaTime)
{
	// 서버에서만 판정
	if (!HasAuthority()) return;

	// 문이 열려있으면 작동하지 않음
	if (bIsDoorOpen) return;
	
	UE_LOG(LogTemp, Warning, TEXT("Incinerator is Burning"));
	
	const float Damage = DamagePerSecond * DeltaTime;
	
	// 소각 영역 내의 컴포넌트 순회
	if (OverlappingBurnables.Num() == 0)
	{
		SetActorTickEnabled(false);
		return;
	}
	for (int32 i = OverlappingBurnables.Num() - 1; i >= 0; --i)
	{
		UBurnableComponent* BurnComp = OverlappingBurnables[i];
        
		// 유효성 검사 (액터가 이미 파괴되었을 수 있음)
		if (BurnComp && BurnComp->GetOwner())
		{
			BurnComp->TakeBurnDamage(Damage);
		}
		else
		{
			// 더 이상 유효하지 않은 컴포넌트는 목록에서 제거
			OverlappingBurnables.RemoveAt(i);
		}
	}
}

void AIncineratorActor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("[Interact] Incinerator Door Move Request Received"));
	
	bIsDoorOpen = !bIsDoorOpen;
	if (bIsDoorOpen) SetActorTickEnabled(false);
	else 
	{
		SetActorTickEnabled(true);
		
		APawn* NoiseInstigator = Cast<APawn>(Interactor);
		
		FVector NoiseLocation = Interactor ? Interactor->GetActorLocation() : GetActorLocation();
		
		UAISense_Hearing::ReportNoiseEvent(this, NoiseLocation, 1.5f, NoiseInstigator);

#if WITH_EDITOR
		if (GetWorld())
		{
			float BaseHearingRange = 2500.0f;
			float SoundRadius = BaseHearingRange * 1.5f;

			DrawDebugSphere(
				GetWorld(),
				NoiseLocation,
				SoundRadius,
				16,
				FColor::Orange,
				false,
				1.0f,
				0,
				2.0f
			);
		}
#endif
	}
    
	OnRep_DoorOpen();
}

void AIncineratorActor::OnRep_DoorOpen()
{
	// 1. 블루프린트에서 추가된 Actor Sequence 컴포넌트를 찾습니다.
	UActorSequenceComponent* SequenceComp = FindComponentByClass<UActorSequenceComponent>();
	if (SequenceComp && SequenceComp->GetSequencePlayer())
	{
		if (bIsDoorOpen)
		{
			// 문을 여는 방향으로 재생
			if (DoorSound1)
			{	
				UGameplayStatics::PlaySoundAtLocation(this, DoorSound1, GetActorLocation()); 
			}
			SequenceComp->GetSequencePlayer()->Play();
		}
		else
		{
			// 문을 닫는 방향(역재생)으로 재생
			if (DoorSound2)
			{	
				UGameplayStatics::PlaySoundAtLocation(this, DoorSound2, GetActorLocation()); 
			}
			SequenceComp->GetSequencePlayer()->PlayReverse();
		}
	}
	
	if (BurningAudioComp)
	{
		if (bIsDoorOpen)
		{
			// 문이 열리면 작동 중지 - 사운드 정지
			BurningAudioComp->FadeOut(0.5f, 0.0f);
		}
		else
		{
			// 문이 닫히면 작동 - 사운드 재생
			BurningAudioComp->FadeIn(0.5f, 1.0f);
		}
	}
}