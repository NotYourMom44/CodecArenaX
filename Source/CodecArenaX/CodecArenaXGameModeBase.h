
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "Respawning")
    float BotRespawnDelay = 5.0f;

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
};
