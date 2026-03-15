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

	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	SessionInterface->CreateSession(0, SessionName, SessionSettings);
}

void UMultiplayGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			// 1. 경로 수정: /Game/ 으로 시작하는지 확인하세요.
			// 2. OpenLevel 사용: 리슨 서버 모드(?listen)로 직접 레벨을 엽니다.
			FString LobbyPath = TEXT("/Game/_CCD/Maps/Lobby");
			
			// 마지막 인자에 "listen"을 넣으면 해당 호스트는 리슨 서버 상태가 됩니다.
			UGameplayStatics::OpenLevel(World, FName(*LobbyPath), true, TEXT("listen"));

			UE_LOG(LogTemp, Warning, TEXT("호스트가 리슨 서버 모드로 로비 이동 중: %s"), *LobbyPath);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("-- 세션 생성 실패 --"));
	}
}

void UMultiplayGameInstance::FindSessions()
{
	if (!SessionInterface.IsValid()) return;

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true; // LAN 환경 테스트
	SessionSearch->MaxSearchResults = 10;
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UMultiplayGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		// 검색된 세션 배열
		TArray<FOnlineSessionSearchResult> SearchResults = SessionSearch->SearchResults;

		UE_LOG(LogTemp, Warning, TEXT("세션 검색 완료! 찾은 방 개수: %d"), SearchResults.Num());

		for (int32 i = 0; i < SearchResults.Num(); i++)
		{
			// 방장 이름 또는 세션 ID 출력
			FString OwnerName = SearchResults[i].Session.OwningUserName;
			int32 Ping = SearchResults[i].PingInMs;

			UE_LOG(LogTemp, Warning, TEXT("방 번호[%d]: 방장(%s), 핑(%d)"), i, *OwnerName, Ping);
            
			// TODO: 여기서 Delegate나 이벤트를 호출하여 UI(UMG) 리스트에 추가 로직을 작성합니다.
		}
	}
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