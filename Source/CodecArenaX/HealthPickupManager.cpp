
#include "HealthPickupManager.h"

#include "HealthPickup.h"
#include "HealthPickupSpawnPoint.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "TimerManager.h"

AHealthPickupManager::AHealthPickupManager()
{
    PrimaryActorTick.bCanEverTick = false;

    PickupCount = 3;
    RespawnDelay = 10.0f;
}

void AHealthPickupManager::BeginPlay()
{
    Super::BeginPlay();

    FindSpawnPoints();

    if (!PickupClass)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("HealthPickupManager: PickupClass is not assigned.")
        );
        return;
    }

    if (AvailableSpawnPoints.Num() < PickupCount)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("HealthPickupManager: Need at least %d enabled spawn points, but found %d."),
            PickupCount,
            AvailableSpawnPoints.Num()
        );
        return;
    }

    SpawnInitialPickups();
}

void AHealthPickupManager::FindSpawnPoints()
{
    AvailableSpawnPoints.Empty();

    for (TActorIterator<AHealthPickupSpawnPoint> It(GetWorld()); It; ++It)
    {
        AHealthPickupSpawnPoint* SpawnPoint = *It;

        if (IsValid(SpawnPoint) && SpawnPoint->bEnabled)
        {
            AvailableSpawnPoints.Add(SpawnPoint);
        }
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("HealthPickupManager: Found %d enabled spawn points."),
        AvailableSpawnPoints.Num()
    );
}

AHealthPickupSpawnPoint* AHealthPickupManager::ChooseSpawnPoint(
    AHealthPickup* Pickup,
    bool bPreferDifferentLocation
) const
{
    TArray<AHealthPickupSpawnPoint*> Candidates;

    for (AHealthPickupSpawnPoint* SpawnPoint : AvailableSpawnPoints)
    {
        if (!IsValid(SpawnPoint))
        {
            continue;
        }

        if (bPreferDifferentLocation)
        {
            const AHealthPickupSpawnPoint* PreviousPoint =
                PreviousSpawnPoints.FindRef(Pickup);

            if (SpawnPoint == PreviousPoint)
            {
                continue;
            }
        }

        bool bOccupied = false;

        for (const TPair<AHealthPickup*, AHealthPickupSpawnPoint*>& Pair
            : CurrentSpawnPoints)
        {
            if (Pair.Key != Pickup && Pair.Value == SpawnPoint)
            {
                bOccupied = true;
                break;
            }
        }

        if (!bOccupied)
        {
            Candidates.Add(SpawnPoint);
        }
    }

    if (Candidates.IsEmpty() && bPreferDifferentLocation)
    {
        return ChooseSpawnPoint(Pickup, false);
    }

    if (Candidates.IsEmpty())
    {
        return nullptr;
    }

    const int32 Index = FMath::RandRange(0, Candidates.Num() - 1);
    return Candidates[Index];
}

void AHealthPickupManager::SpawnInitialPickups()
{
    for (int32 Index = 0; Index < PickupCount; ++Index)
    {
        AHealthPickupSpawnPoint* SpawnPoint = ChooseSpawnPoint(nullptr, false);

        if (!SpawnPoint)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT("HealthPickupManager: Could not choose a spawn point.")
            );
            return;
        }

        const FTransform SpawnTransform = SpawnPoint->GetActorTransform();

        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AHealthPickup* Pickup = GetWorld()->SpawnActor<AHealthPickup>(
            PickupClass,
            SpawnTransform,
            SpawnParameters
        );

        if (!Pickup)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT("HealthPickupManager: Failed to spawn a pickup.")
            );
            continue;
        }

        Pickup->SetManagerControlled(true);
        Pickup->OnCollected.AddDynamic(
            this,
            &AHealthPickupManager::HandlePickupCollected
        );

        Pickup->ActivateAtLocation(SpawnPoint->GetActorLocation());

        CurrentSpawnPoints.Add(Pickup, SpawnPoint);

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("HealthPickupManager: Spawned pickup %s at %s"),
            *Pickup->GetName(),
            *SpawnPoint->GetName()
        );
    }
}

void AHealthPickupManager::HandlePickupCollected(
    AHealthPickup* CollectedPickup
)
{
    if (!IsValid(CollectedPickup))
    {
        return;
    }

    AHealthPickupSpawnPoint* PreviousPoint =
        CurrentSpawnPoints.FindRef(CollectedPickup);

    if (PreviousPoint)
    {
        PreviousSpawnPoints.Add(CollectedPickup, PreviousPoint);
    }

    CurrentSpawnPoints.Remove(CollectedPickup);

    FTimerHandle& TimerHandle = RespawnTimers.FindOrAdd(CollectedPickup);

    GetWorldTimerManager().SetTimer(
        TimerHandle,
        FTimerDelegate::CreateUObject(
            this,
            &AHealthPickupManager::RespawnCollectedPickup,
            CollectedPickup
        ),
        RespawnDelay,
        false
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("HealthPickupManager: Pickup collected; respawning in %.1f seconds."),
        RespawnDelay
    );
}

void AHealthPickupManager::RespawnCollectedPickup(AHealthPickup* Pickup)
{
    if (!IsValid(Pickup))
    {
        RespawnTimers.Remove(Pickup);
        return;
    }

    AHealthPickupSpawnPoint* SpawnPoint = ChooseSpawnPoint(
        Pickup,
        true
    );

    if (!SpawnPoint)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("HealthPickupManager: No available spawn point for respawn.")
        );
        return;
    }

    Pickup->ActivateAtLocation(SpawnPoint->GetActorLocation());

    CurrentSpawnPoints.Add(Pickup, SpawnPoint);
    RespawnTimers.Remove(Pickup);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("HealthPickupManager: Respawned %s at %s"),
        *Pickup->GetName(),
        *SpawnPoint->GetName()
    );
}
