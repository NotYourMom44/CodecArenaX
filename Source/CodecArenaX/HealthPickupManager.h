
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HealthPickupManager.generated.h"

class AHealthPickup;
class AHealthPickupSpawnPoint;

UCLASS()
class CODECARENAX_API AHealthPickupManager : public AActor
{
    GENERATED_BODY()

public:
    AHealthPickupManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health Pickups")
    TSubclassOf<AHealthPickup> PickupClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health Pickups", meta = (ClampMin = "1"))
    int32 PickupCount;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health Pickups", meta = (ClampMin = "0.1"))
    float RespawnDelay;

private:
    TArray<AHealthPickupSpawnPoint*> AvailableSpawnPoints;

    TMap<AHealthPickup*, AHealthPickupSpawnPoint*> CurrentSpawnPoints;

    TMap<AHealthPickup*, AHealthPickupSpawnPoint*> PreviousSpawnPoints;

    TMap<AHealthPickup*, FTimerHandle> RespawnTimers;

    void FindSpawnPoints();

    AHealthPickupSpawnPoint* ChooseSpawnPoint(
        AHealthPickup* Pickup,
        bool bPreferDifferentLocation
    ) const;

    void SpawnInitialPickups();

    UFUNCTION()
    void HandlePickupCollected(AHealthPickup* CollectedPickup);

    void RespawnCollectedPickup(AHealthPickup* Pickup);
};
