#include "MultiplayGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"

UMultiplayGameInstance::UMultiplayGameInstance()
{
	// 데리게이트와 함수 연결
	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayGameInstance::OnCreateSessionComplete);
	FindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMultiplayGameInstance::OnFindSessionsComplete);
	JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiplayGameInstance::OnJoinSessionComplete);
}

void UMultiplayGameInstance::Init()
{
	Super::Init();

	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
}

void UMultiplayGameInstance::HostSession(FName SessionName, bool bIsLAN)
{
	if (!SessionInterface.IsValid()) return;

	UE_LOG(LogTemp, Warning, TEXT("HostSession 시도: %s, LAN: %s"), *SessionName.ToString(), bIsLAN ? TEXT("Yes") : TEXT("No"));
	
	// 기존 세션이 있다면 삭제 (생략 가능하나 안전을 위해 권장)
	auto ExistingSession = SessionInterface->GetNamedSession(SessionName);
	if (ExistingSession)
	{
		SessionInterface->DestroySession(SessionName);
	}

	// 세션 설정
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = bIsLAN;
	SessionSettings.NumPublicConnections = 3; // 최대 인원
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bShouldAdvertise = true; // 서버 리스트에 노출 여부
	SessionSettings.bUsesPresence = true;    // Steam 등에서 상태 표시 사용

	SessionSettings.bIsDedicated = false;
	SessionSettings.bUsesStats = false;
	
	// 세션 데이터에 커스텀 이름 저장 (필요 시)
	// SessionSettings.Set(SETTING_MAPNAME, FString("Lobby"), EOnlineDataAdvertisementType::ViaOnlineService);
	
	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	SessionInterface->CreateSession(0, SessionName, SessionSettings);
}

void UMultiplayGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (UWorld* World = GetWorld())
		{
			FString LobbyPath = TEXT("/Game/_CCD/Maps/Lobby");
			UGameplayStatics::OpenLevel(World, FName(*LobbyPath), true, TEXT("listen"));
			UE_LOG(LogTemp, Warning, TEXT("세션 생성 성공: %s"), *SessionName.ToString());
		}
	}
}

void UMultiplayGameInstance::FindSessions()
{
	if (!SessionInterface.IsValid()) return;

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	
	SessionSearch->bIsLanQuery = true; // LAN 환경 테스트
	SessionSearch->MaxSearchResults = 50;
	
	// SEARCH PRESENCE
	// SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCE")), true, EOnlineComparisonOp::Equals);

	SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	
	UE_LOG(LogTemp, Warning, TEXT("방 찾기 시작..."));
}

void UMultiplayGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	TArray<FString> SessionNames;
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("방 검색 성공. 개수: %d"), SessionSearch->SearchResults.Num());
		for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
		{
			// UI 리스트에 표시할 이름 추가 (방장 이름)
			SessionNames.Add(Result.Session.OwningUserName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("방 검색 실패 또는 결과 없음."));
	}
	OnFindSessionsCompleteEvent.Broadcast(SessionNames);
}

void UMultiplayGameInstance::JoinGameSession(int32 SessionIndex)
{
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid()) return;

	if (SessionSearch->SearchResults.Num() > SessionIndex)
	{
		SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
		SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[SessionIndex]);
	}
}

void UMultiplayGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (APlayerController* PC = GetFirstLocalPlayerController())
		{
			FString ConnectAddress;
			if (SessionInterface->GetResolvedConnectString(SessionName, ConnectAddress))
			{
				PC->ClientTravel(ConnectAddress, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}