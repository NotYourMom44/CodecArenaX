#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "CodecArenaXGameModeBase.generated.h"

class ABotCharacter;
class ABaseCharacter;
class AController;

UCLASS()
class CODECARENAX_API ACodecArenaXGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACodecArenaXGameModeBase();

    UFUNCTION(BlueprintCallable, Category = "Respawning")
    AActor* ChooseRandomSpawnPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Respawning")
    void HandleBotDeath(ABotCharacter* DeadBot);

    UFUNCTION(BlueprintCallable, Category = "Respawning")
    void HandlePlayerDeath(AController* DeadController);

    UFUNCTION(BlueprintCallable, Category = "Match")
    void StartMatch();

    UFUNCTION(BlueprintCallable, Category = "Match")
    void EndMatch();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "Respawning")
    float BotRespawnDelay = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Match")
    int32 MatchDurationSeconds = 300;

private:
    void RegisterInitialParticipants();

    void AwardKillForDeath(
        ABaseCharacter* Victim,
        AController* VictimController
    );

    void RespawnBot(
        TSubclassOf<ABotCharacter> BotClass,
        FName ParticipantId
    );

    void RespawnPlayer(AController* Controller);

    void UpdateMatchTimer();

    FTimerHandle MatchTimerHandle;

    int32 RemainingMatchSeconds = 300;

    bool bMatchInProgress = false;

    bool bMatchEnded = false;
};