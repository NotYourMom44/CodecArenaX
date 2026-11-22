
#include "CodecArenaXGameModeBase.h"

#include "GameFramework/Controller.h"
#include "BotCharacter.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"

AActor* ACodecArenaXGameModeBase::ChooseRandomSpawnPoint() const
{
    TArray<APlayerStart*> SpawnPoints;

    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        SpawnPoints.Add(*It);
    }

    if (SpawnPoints.IsEmpty())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Respawning: No PlayerStart actors found."));
        return nullptr;
    }

    const int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
    return SpawnPoints[RandomIndex];
}

void ACodecArenaXGameModeBase::HandleBotDeath(ABotCharacter* DeadBot)
{
    if (!IsValid(DeadBot))
    {
        return;
    }

    TSubclassOf<ABotCharacter> BotClass = DeadBot->GetClass();

    // Capture the participant ID before destroying the bot.
    FName ParticipantId = DeadBot->GetParticipantId();

    if (ParticipantId.IsNone())
    {
        ParticipantId = DeadBot->GetFName();
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Respawning: Bot %s (Participant ID: %s) will respawn in %.1f seconds."),
        *GetNameSafe(DeadBot),
        *ParticipantId.ToString(),
        BotRespawnDelay
    );

    DeadBot->Destroy();

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
    if (!BotClass)
    {
        return;
    }

    AActor* SpawnPoint = ChooseRandomSpawnPoint();

    if (!SpawnPoint)
    {
        UE_LOG(LogTemp, Error,
            TEXT("Respawning: Cannot respawn bot because no PlayerStart exists."));
        return;
    }

    const FTransform SpawnTransform = SpawnPoint->GetActorTransform();

    // Spawn the bot in a deferred state so its original identity
    // can be assigned before BeginPlay runs.
    ABotCharacter* NewBot = GetWorld()->SpawnActorDeferred<ABotCharacter>(
        BotClass,
        SpawnTransform,
        nullptr,
        nullptr,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
    );

    if (!NewBot)
    {
        UE_LOG(LogTemp, Error,
            TEXT("Respawning: Failed to create replacement bot."));
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

void ACodecArenaXGameModeBase::HandlePlayerDeath(
    AController* DeadController)
{
    if (!IsValid(DeadController))
    {
        return;
    }

    APawn* DeadPawn = DeadController->GetPawn();

    if (IsValid(DeadPawn))
    {
        DeadPawn->Destroy();
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

void ACodecArenaXGameModeBase::RespawnPlayer(
    AController* Controller)
{
    if (!IsValid(Controller))
    {
        return;
    }

    RestartPlayer(Controller);
}
