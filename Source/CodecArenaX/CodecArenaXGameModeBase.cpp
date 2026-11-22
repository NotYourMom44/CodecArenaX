
#include "CodecArenaXGameModeBase.h"

#include "CodecArenaXGameState.h"
#include "BaseCharacter.h"
#include "BotCharacter.h"
#include "HealthComponent.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"

ACodecArenaXGameModeBase::ACodecArenaXGameModeBase()
{
    GameStateClass = ACodecArenaXGameState::StaticClass();
}

void ACodecArenaXGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    GetWorld()->GetTimerManager().SetTimerForNextTick(
        this,
        &ACodecArenaXGameModeBase::RegisterInitialParticipants
    );
}

void ACodecArenaXGameModeBase::RegisterInitialParticipants()
{
    ACodecArenaXGameState* ScoringGameState =
        GetGameState<ACodecArenaXGameState>();

    if (!ScoringGameState)
    {
        UE_LOG(LogTemp, Error, TEXT("Scoring: Custom GameState could not be found."));
        return;
    }

    ScoringGameState->RegisterParticipant(
        FName("Player"),
        TEXT("Player"),
        true
    );

    int32 RegisteredBotCount = 0;

    for (TActorIterator<ABotCharacter> It(GetWorld()); It; ++It)
    {
        ABotCharacter* Bot = *It;

        if (!IsValid(Bot))
        {
            continue;
        }

        FName ParticipantId = Bot->GetParticipantId();

        if (ParticipantId.IsNone())
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("Scoring: Bot %s has no participant ID yet."),
                *GetNameSafe(Bot)
            );

            continue;
        }

        ScoringGameState->RegisterParticipant(
            ParticipantId,
            ParticipantId.ToString(),
            false
        );

        RegisteredBotCount++;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Scoring: Initial registration complete. Registered %d bots."),
        RegisteredBotCount
    );

    // Start the match after all initial participants are registered.
    StartMatch();
}

void ACodecArenaXGameModeBase::StartMatch()
{
    if (bMatchInProgress)
    {
        return;
    }

    ACodecArenaXGameState* MatchGameState =
        GetGameState<ACodecArenaXGameState>();

    if (!MatchGameState)
    {
        UE_LOG(LogTemp, Error, TEXT("Match: Cannot start because GameState is unavailable."));
        return;
    }

    bMatchEnded = false;
    bMatchInProgress = true;

    RemainingMatchSeconds = FMath::Max(1, MatchDurationSeconds);

    MatchGameState->ResetScores();
    MatchGameState->SetRemainingMatchTime(RemainingMatchSeconds);
    MatchGameState->SetMatchState(ECodecArenaMatchState::InProgress);

    GetWorld()->GetTimerManager().ClearTimer(MatchTimerHandle);

    GetWorld()->GetTimerManager().SetTimer(
        MatchTimerHandle,
        this,
        &ACodecArenaXGameModeBase::UpdateMatchTimer,
        1.0f,
        true
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Match: Started. Duration: %d seconds."),
        RemainingMatchSeconds
    );
}

void ACodecArenaXGameModeBase::UpdateMatchTimer()
{
    if (!bMatchInProgress)
    {
        return;
    }

    RemainingMatchSeconds--;

   

    ACodecArenaXGameState* MatchGameState =
        GetGameState<ACodecArenaXGameState>();

    if (MatchGameState)
    {
        MatchGameState->SetRemainingMatchTime(RemainingMatchSeconds);
    }

    if (RemainingMatchSeconds <= 0)
    {
        EndMatch();
    }
}

void ACodecArenaXGameModeBase::EndMatch()
{
    if (bMatchEnded)
    {
        return;
    }

    bMatchInProgress = false;
    bMatchEnded = true;

    GetWorld()->GetTimerManager().ClearTimer(MatchTimerHandle);

    ACodecArenaXGameState* MatchGameState =
        GetGameState<ACodecArenaXGameState>();

    if (MatchGameState)
    {
        MatchGameState->SetRemainingMatchTime(0);
        MatchGameState->SetMatchState(ECodecArenaMatchState::MatchOver);
    }

    UE_LOG(LogTemp, Warning, TEXT("Match: Time expired. Match over."));
}

void ACodecArenaXGameModeBase::AwardKillForDeath(
    ABaseCharacter* Victim,
    AController* VictimController
)
{
    if (!bMatchInProgress || !IsValid(Victim))
    {
        return;
    }

    UHealthComponent* VictimHealth = Victim->GetHealthComponent();

    if (!VictimHealth)
    {
        return;
    }

    AController* KillerController =
        VictimHealth->GetLastDamageInstigator();

    if (!IsValid(KillerController))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Scoring: No valid killer found for %s. No kill awarded."),
            *GetNameSafe(Victim)
        );

        return;
    }

    if (KillerController == VictimController)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Scoring: Self-inflicted death for %s. No kill awarded."),
            *GetNameSafe(Victim)
        );

        return;
    }

    FName KillerParticipantId = NAME_None;

    if (Cast<APlayerController>(KillerController))
    {
        KillerParticipantId = FName("Player");
    }
    else
    {
        APawn* KillerPawn = KillerController->GetPawn();

        ABotCharacter* KillerBot = Cast<ABotCharacter>(KillerPawn);

        if (KillerBot)
        {
            KillerParticipantId = KillerBot->GetParticipantId();
        }
    }

    if (KillerParticipantId.IsNone())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Scoring: Could not identify killer for %s. No kill awarded."),
            *GetNameSafe(Victim)
        );

        return;
    }

    ACodecArenaXGameState* ScoringGameState =
        GetGameState<ACodecArenaXGameState>();

    if (!ScoringGameState)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Scoring: Cannot award kill because GameState is unavailable.")
        );

        return;
    }

    ScoringGameState->AddKill(KillerParticipantId);
}

AActor* ACodecArenaXGameModeBase::ChooseRandomSpawnPoint() const
{
    TArray<APlayerStart*> SpawnPoints;

    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        SpawnPoints.Add(*It);
    }

    if (SpawnPoints.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Respawning: No PlayerStart actors found.")
        );

        return nullptr;
    }

    const int32 RandomIndex =
        FMath::RandRange(0, SpawnPoints.Num() - 1);

    return SpawnPoints[RandomIndex];
}

void ACodecArenaXGameModeBase::HandleBotDeath(ABotCharacter* DeadBot)
{
    if (!IsValid(DeadBot))
    {
        return;
    }

    AwardKillForDeath(DeadBot, DeadBot->GetController());

    TSubclassOf<ABotCharacter> BotClass = DeadBot->GetClass();

    FName ParticipantId = DeadBot->GetParticipantId();

    if (ParticipantId.IsNone())
    {
        ParticipantId = DeadBot->GetFName();
    }

    DeadBot->Destroy();

    if (!bMatchInProgress)
    {
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Respawning: Bot %s (Participant ID: %s) will respawn in %.1f seconds."),
        *GetNameSafe(DeadBot),
        *ParticipantId.ToString(),
        BotRespawnDelay
    );

    FTimerHandle RespawnTimer;
    FTimerDelegate RespawnDelegate;

    RespawnDelegate.BindUObject(
        this,
        &ACodecArenaXGameModeBase::RespawnBot,
        BotClass,
        ParticipantId
    );

    GetWorld()->GetTimerManager().SetTimer(
        RespawnTimer,
        RespawnDelegate,
        BotRespawnDelay,
        false
    );
}

void ACodecArenaXGameModeBase::RespawnBot(
    TSubclassOf<ABotCharacter> BotClass,
    FName ParticipantId
)
{
    if (!bMatchInProgress || !BotClass)
    {
        return;
    }

    AActor* SpawnPoint = ChooseRandomSpawnPoint();

    if (!SpawnPoint)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Respawning: Cannot respawn bot because no PlayerStart exists.")
        );

        return;
    }

    const FTransform SpawnTransform = SpawnPoint->GetActorTransform();

    ABotCharacter* NewBot = GetWorld()->SpawnActorDeferred<ABotCharacter>(
        BotClass,
        SpawnTransform,
        nullptr,
        nullptr,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
    );

    if (!NewBot)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Respawning: Failed to create replacement bot.")
        );

        return;
    }

    NewBot->SetParticipantId(ParticipantId);
    NewBot->FinishSpawning(SpawnTransform);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Respawning: Spawned replacement bot %s with Participant ID %s."),
        *GetNameSafe(NewBot),
        *NewBot->GetParticipantId().ToString()
    );
}

void ACodecArenaXGameModeBase::HandlePlayerDeath(AController* DeadController)
{
    if (!IsValid(DeadController))
    {
        return;
    }

    APawn* DeadPawn = DeadController->GetPawn();

    if (IsValid(DeadPawn))
    {
        ABaseCharacter* DeadCharacter = Cast<ABaseCharacter>(DeadPawn);

        if (DeadCharacter)
        {
            AwardKillForDeath(DeadCharacter, DeadController);
        }

        DeadPawn->Destroy();
    }

    if (!bMatchInProgress)
    {
        return;
    }

    FTimerHandle RespawnTimer;
    FTimerDelegate RespawnDelegate;

    RespawnDelegate.BindUObject(
        this,
        &ACodecArenaXGameModeBase::RespawnPlayer,
        DeadController
    );

    GetWorld()->GetTimerManager().SetTimer(
        RespawnTimer,
        RespawnDelegate,
        BotRespawnDelay,
        false
    );
}

void ACodecArenaXGameModeBase::RespawnPlayer(AController* Controller)
{
    if (!bMatchInProgress || !IsValid(Controller))
    {
        return;
    }

    RestartPlayer(Controller);
}
