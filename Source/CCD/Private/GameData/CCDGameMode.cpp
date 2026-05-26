#include "GameData/CCDGameMode.h"
#include "Player/CCDCharacter.h"
#include "Player/CCDPlayerController.h"
#include "Actor/ProgressManager.h"
#include "Actor/SharedLivesManager.h"
#include "GameData/CCDSaveGame.h"
#include "GameData/CCDGameRecordSubsystem.h"
#include "GameData/CCDGameState.h"
#include "Kismet/GameplayStatics.h"

#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"

ACCDGameMode::ACCDGameMode()
{
	// 심리스 트래블을 활성화하여 네트워크 연결을 유지한 채 맵을 이동합니다.
	bUseSeamlessTravel = true;
}

void ACCDGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 월드에서 ProgressManager를 찾아 보관합니다.
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AProgressManager::StaticClass());
	ProgressManager = Cast<AProgressManager>(FoundActor);
	
	if (!HasAuthority()) return; // 서버에서만 실행
	
	// 서버에서 게임이 시작될 때, 현재 세션의 클리어된 맵 목록을 GameState의 복제 배열에 반영
	if (UCCDGameRecordSubsystem* RecordSystem = GetGameInstance()->GetSubsystem<UCCDGameRecordSubsystem>())
	{
		if (ACCDGameState* GS = GetGameState<ACCDGameState>())
		{
			// 힌트: 서브시스템에 현재 활성화된 세션의 클리어 맵 목록만 가져오는 함수를 추가하면 편리합니다.
			TMap<FString, bool> ClearedMaps = RecordSystem->GetCurrentSessionClearedMaps(); 
			for (const auto& Map : ClearedMaps)
			{
				FString MapPath = UWorld::RemovePIEPrefix(Map.Key);
				GS->ReplicatedClearedMapPaths.AddUnique(MapPath);
			}
		}
	}

	// 로비 맵을 제외한 일반 인게임 맵인 경우 중간 난입을 차단합니다.
	FString CurrentMapName = GetWorld()->GetOutermost()->GetName();
	CurrentMapName = UWorld::RemovePIEPrefix(CurrentMapName);
	UE_LOG(LogTemp, Warning, TEXT("[CCDGameMode] Current Map Name: %s"), *CurrentMapName);

	// 로비 맵 이름(예: "TUWorld")이 아닐 때만 차단 로직 실행
	if (CurrentMapName != TEXT("/Game/Maps/TUWorld"))
	{
		SetJoinInProgressAllowed(false);
	}
	else 
	{
		SetJoinInProgressAllowed(true);
	}
}

void ACCDGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	
	FString CurrentMapName = GetWorld()->GetOutermost()->GetName();
	CurrentMapName = UWorld::RemovePIEPrefix(CurrentMapName);

	// 로비가 아닌 실제 청소 레벨인데 외부 인원이 들어오려고 하면 튕겨냅니다.
	if (CurrentMapName != TEXT("/Game/Maps/TUWorld"))
	{
		ErrorMessage = TEXT("The game is already in progress.");
		UE_LOG(LogTemp, Warning, TEXT("Blocked mid-game join attempt from: %s"), *Address);
	}
}

void ACCDGameMode::OnCleaningFinished()
{
	if (ACCDGameState* GS = GetGameState<ACCDGameState>())
	{
		GS->bIsCleaningFinished = true;
		GS->OnRep_CleaningFinished(); 
	}
	
	// 맵이 클리어 됐다는 사실을 기록
	if (UCCDGameRecordSubsystem* RecordSystem = GetGameInstance()->GetSubsystem<UCCDGameRecordSubsystem>())
	{
		FString CurrentMapName = GetWorld()->GetOutermost()->GetName();
		CurrentMapName = UWorld::RemovePIEPrefix(CurrentMapName);
		RecordSystem->RecordMapClear(CurrentMapName);
		
		// 서버에서 GameState의 복제 배열 업데이트
		if (ACCDGameState* GS = GetGameState<ACCDGameState>())
		{
			GS->ReplicatedClearedMapPaths.AddUnique(CurrentMapName);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("GameMode : Cleaning Finished!"));
}

void ACCDGameMode::RequestRespawn(ACCDCharacter* DeadCharacter)
{
	// 월드에서 매니저를 찾아 부활 가능 여부를 확인합니다.
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASharedLivesManager::StaticClass());
	ASharedLivesManager* LivesManager = Cast<ASharedLivesManager>(FoundActor);

	if (LivesManager && LivesManager->AttemptDecrementLife())
	{
		DeadCharacter->Revive();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No more lives or Manager not found!"));
	}
}

void ACCDGameMode::TransitionToLevel(const FString& NextLevelPath)
{
	if (NextLevelPath.IsEmpty()) return;

	// 1. 모든 접속된 플레이어 컨트롤러를 순회하며 로딩 UI 출력을 명령합니다.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ACCDPlayerControllerBase* PC = Cast<ACCDPlayerControllerBase>(It->Get()))
		{
			PC->Client_StartLoading(); // RPC 호출
		}
	}

	// 2. 클라이언트가 UI를 띄우고 네트워크 패킷을 처리할 시간을 준 뒤 이동합니다.
	FTimerHandle TravelTimer;
	GetWorldTimerManager().SetTimer(TravelTimer, [this, NextLevelPath]()
	{
		// 심리스 트래블 활성 상태이므로 연결을 유지하며 이동합니다.
		GetWorld()->ServerTravel(NextLevelPath);
	}, 1.0f, false);
}

int32 ACCDGameMode::GetCurrentLives() const
{
	// 월드에서 매니저를 찾아 부활 가능 여부를 확인합니다.
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASharedLivesManager::StaticClass());
	if (ASharedLivesManager* LivesManager = Cast<ASharedLivesManager>(FoundActor))
	{
		return LivesManager->GetCurrentLives();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No more lives or Manager not found!"));
	}
	
	return -1; // 매니저가 없거나 오류 시 -1 반환
}

void ACCDGameMode::SetJoinInProgressAllowed(bool bAllowJoin)
{
	if (!HasAuthority()) return;

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// 현재 열려있는 세션 설정 가져오기
			if (FOnlineSessionSettings* CurrentSettings = SessionInterface->GetSessionSettings(NAME_GameSession))
			{
				CurrentSettings->bAllowJoinInProgress = bAllowJoin;

				// 세션 업데이트를 서버 및 플랫폼(스팀/LAN)에 반영합니다.
				SessionInterface->UpdateSession(NAME_GameSession, *CurrentSettings, true);
				UE_LOG(LogTemp, Warning, TEXT("[CCDGameMode] bAllowJoinInProgress set to: %s"), bAllowJoin ? TEXT("True") : TEXT("False"));
			}
		}
	}
}