
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CodecArenaXGameModeBase.generated.h"

class ABotCharacter;
class AController;

UCLASS()
class CODECARENAX_API ACodecArenaXGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Respawning")
    AActor* ChooseRandomSpawnPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Respawning")
    void HandleBotDeath(ABotCharacter* DeadBot);

    UFUNCTION(BlueprintCallable, Category = "Respawning")
    void HandlePlayerDeath(AController* DeadController);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Respawning")
    float BotRespawnDelay = 5.0f;

private:
    void RespawnBot(TSubclassOf<ABotCharacter> BotClass);
    void RespawnPlayer(AController* Controller);
};
