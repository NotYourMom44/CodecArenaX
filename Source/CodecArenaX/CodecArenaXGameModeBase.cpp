
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

    UE_LOG(LogTemp, Warning,
        TEXT("Respawning: %s will respawn in %.1f seconds."),
        *GetNameSafe(DeadBot), BotRespawnDelay);

    DeadBot->Destroy();

    FTimerHandle RespawnTimer;
    FTimerDelegate RespawnDelegate;
    RespawnDelegate.BindUObject(
        this,
        &ACodecArenaXGameModeBase::RespawnBot,
        BotClass
    );

    GetWorld()->GetTimerManager().SetTimer(
        RespawnTimer,
        RespawnDelegate,
        BotRespawnDelay,
        false
    );
}

void ACodecArenaXGameModeBase::RespawnBot(
    TSubclassOf<ABotCharacter> BotClass)
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

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ABotCharacter* NewBot = GetWorld()->SpawnActor<ABotCharacter>(
        BotClass,
        SpawnPoint->GetActorTransform(),
        SpawnParameters
    );

    if (NewBot)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Respawning: Spawned replacement bot %s."),
            *GetNameSafe(NewBot));
    }
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
