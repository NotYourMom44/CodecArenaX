#include "BotAIController.h"

#include "HealthComponent.h"
#include "HealthPickup.h"
#include "WeaponAmmoPickup.h"
#include "WeaponInventoryComponent.h"
#include "SecondaryWeapon.h"
#include "BaseWeapon.h"
#include "SecondaryWeapon.h"
#include "BotCharacter.h"
#include "PlayerCharacter.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

ABotAIController::ABotAIController()
{
    AIPerception =
        CreateDefaultSubobject<UAIPerceptionComponent>(
            TEXT("AIPerception")
        );

    SightConfig =
        CreateDefaultSubobject<UAISenseConfig_Sight>(
            TEXT("SightConfig")
        );

    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 70.0f;

    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    AIPerception->ConfigureSense(*SightConfig);
    AIPerception->SetDominantSense(
        UAISense_Sight::StaticClass()
    );

    AIPerception->OnTargetPerceptionUpdated.AddDynamic(
        this,
        &ABotAIController::HandleTargetPerceptionUpdated
    );
}

void ABotAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("BotAIController possessed: %s"),
        *GetNameSafe(InPawn)
    );

    // Snapshot 1: state at possession time.
    LogNavigationRegistrationState(TEXT("AT POSSESS"));

    // Snapshot 2: one-shot, about 2 seconds later. Read-only.
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            NavDiagnosticTimerHandle,
            this,
            &ABotAIController::LogDelayedNavigationState,
            NavDiagnosticDelaySeconds,
            false,
            -1.0f
        );
    }

    MoveToRandomPatrolPoint();
}

void ABotAIController::LogDelayedNavigationState()
{
    // Capture the navigation state after startup.
    LogNavigationRegistrationState(TEXT("AFTER DELAY"));

    // Retry patrol now that navigation data may be registered.
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("PATROL RETRY: Retrying patrol for %s"),
        *GetNameSafe(GetPawn())
    );

    MoveToRandomPatrolPoint();
}

void ABotAIController::LogNavigationRegistrationState(const TCHAR* Context)
{
    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("NAV REG [%s]: World not found."), Context);
        return;
    }

    UNavigationSystemV1* NavigationSystem =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("NAV REG [%s]: time=%.3f Pawn=%s NavigationSystem=%s"),
        Context,
        World->GetTimeSeconds(),
        *GetNameSafe(GetPawn()),
        NavigationSystem ? TEXT("VALID") : TEXT("NULL")
    );

    if (!NavigationSystem)
    {
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("NAV REG [%s]: HasBegunPlay=%s NavSysInitialized=%s WorldInitDone=%s"),
        Context,
        World->HasBegunPlay() ? TEXT("YES") : TEXT("NO"),
        NavigationSystem->IsInitialized() ? TEXT("YES") : TEXT("NO"),
        NavigationSystem->IsWorldInitDone() ? TEXT("YES") : TEXT("NO")
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("NAV REG [%s]: Registered (NavDataSet)=%d | Pending (RegistrationQueue)=%d | MainNavData=%s"),
        Context,
        NavigationSystem->NavDataSet.Num(),
        NavigationSystem->NavDataRegistrationQueue.Num(),
        *GetNameSafe(NavigationSystem->MainNavData.Get())
    );

    for (int32 Index = 0; Index < NavigationSystem->NavDataSet.Num(); ++Index)
    {
        const ANavigationData* RegisteredNavData =
            NavigationSystem->NavDataSet[Index].Get();

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("NAV REG [%s]:   Registered[%d] Name=%s Class=%s"),
            Context,
            Index,
            *GetNameSafe(RegisteredNavData),
            *GetNameSafe(RegisteredNavData ? RegisteredNavData->GetClass() : nullptr)
        );
    }

    for (int32 Index = 0; Index < NavigationSystem->NavDataRegistrationQueue.Num(); ++Index)
    {
        const ANavigationData* PendingNavData =
            NavigationSystem->NavDataRegistrationQueue[Index].Get();

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("NAV REG [%s]:   Pending[%d] Name=%s Class=%s"),
            Context,
            Index,
            *GetNameSafe(PendingNavData),
            *GetNameSafe(PendingNavData ? PendingNavData->GetClass() : nullptr)
        );
    }

    int32 ExistingNavDataActors = 0;

    for (TActorIterator<ANavigationData> It(World); It; ++It)
    {
        ANavigationData* ExistingNavData = *It;
        ++ExistingNavDataActors;

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("NAV REG [%s]:   Existing actor Name=%s Class=%s InRegisteredSet=%s InPendingQueue=%s"),
            Context,
            *GetNameSafe(ExistingNavData),
            *GetNameSafe(ExistingNavData ? ExistingNavData->GetClass() : nullptr),
            NavigationSystem->NavDataSet.Contains(ExistingNavData) ? TEXT("YES") : TEXT("NO"),
            NavigationSystem->NavDataRegistrationQueue.Contains(ExistingNavData) ? TEXT("YES") : TEXT("NO")
        );
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("NAV REG [%s]: Existing ANavigationData actors in PIE world=%d"),
        Context,
        ExistingNavDataActors
    );

    // The same lookup the patrol code uses, repeated read-only.
    const ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());

    if (ControlledCharacter)
    {
        const FNavAgentProperties& AgentProperties =
            ControlledCharacter->GetNavAgentPropertiesRef();

        const ANavigationData* LookupResult =
            NavigationSystem->GetNavDataForProps(
                AgentProperties,
                ControlledCharacter->GetActorLocation(),
                FVector(100.0f, 100.0f, 200.0f)
            );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("NAV REG [%s]: GetNavDataForProps -> %s | AgentRadius=%.1f AgentHeight=%.1f | SupportedAgents=%d SupportedAgentIndex=%d"),
            Context,
            *GetNameSafe(LookupResult),
            AgentProperties.AgentRadius,
            AgentProperties.AgentHeight,
            NavigationSystem->GetSupportedAgents().Num(),
            NavigationSystem->GetSupportedAgentIndex(AgentProperties)
        );
    }
    else
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("NAV REG [%s]: Pawn is not an ACharacter; skipped lookup."),
            Context
        );
    }
}



void ABotAIController::OnMoveCompleted(
    FAIRequestID RequestID,
    const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    // Retreat movement takes priority over patrol and pursuit.
    if (bIsRetreating)
    {
        if (Result.Code == EPathFollowingResult::Success)
        {
            bRetreatRetryUsed = false;
            RetreatFromCombat();
        }
        else
        {
            const UWorld* World = GetWorld();
            AActor* RetreatLogTarget = CurrentTarget.Get();

            UE_LOG(
                LogTemp,
                Warning,
                TEXT("RETREAT MOVE DID NOT SUCCEED: Time=%.3f | Pawn=%s | RequestID=%u | Result=%s | Code=%d | Flags=%d | Retreating=%s | RetryUsed=%s | Location=%s | Target=%s | TargetLocation=%s"),
                World ? World->GetTimeSeconds() : -1.0f,
                *GetNameSafe(GetPawn()),
                RequestID.GetID(),
                *Result.ToString(),
                static_cast<int32>(Result.Code),
                static_cast<int32>(Result.Flags),
                bIsRetreating ? TEXT("YES") : TEXT("NO"),
                bRetreatRetryUsed ? TEXT("YES") : TEXT("NO"),
                *GetPawn()->GetActorLocation().ToString(),
                *GetNameSafe(RetreatLogTarget),
                IsValid(RetreatLogTarget)
                ? *RetreatLogTarget->GetActorLocation().ToString()
                : TEXT("None")
            );


            if (Result.Code == EPathFollowingResult::Blocked &&
                bIsRetreating &&
                IsLowOnHealth())
            {
                UE_LOG(
                    LogTemp,
                    Warning,
                    TEXT("RETREAT BLOCKED: Trying another destination for %s"),
                    *GetNameSafe(GetPawn())
                );

                RetreatFromCombat();
            }
            else if (Result.Code == EPathFollowingResult::Aborted &&
                !bRetreatRetryUsed &&
                bIsRetreating &&
                IsLowOnHealth())
            {
                bRetreatRetryUsed = true;
                RetreatFromCombat();
            }

        }

        return;
    }

    // Handle movement that was targeting a health pickup.
    if (bSeekingHealthPickup)
    {
        AHealthPickup* Pickup = CurrentHealthPickup.Get();

        // Stop seeking if the pickup disappeared or became inactive.
        if (!IsValid(Pickup) ||
            Pickup->IsHidden() ||
            !Pickup->GetActorEnableCollision())
        {
            bSeekingHealthPickup = false;
            CurrentHealthPickup.Reset();

            if (Result.Code == EPathFollowingResult::Success)
            {
                TrySeekHealthPickup();

                if (!bSeekingHealthPickup)
                {
                    MoveToRandomPatrolPoint();
                }
            }

            return;
        }

        // Reached the pickup's destination; allow overlap to heal the bot.
        if (Result.Code == EPathFollowingResult::Success)
        {
            bSeekingHealthPickup = false;
            CurrentHealthPickup.Reset();

            TrySeekHealthPickup();

            if (!bSeekingHealthPickup)
            {
                MoveToRandomPatrolPoint();
            }
        }
        else
        {
            // Avoid immediately selecting the same unreachable pickup.
            FailedHealthPickup = Pickup;

            UE_LOG(
                LogTemp,
                Warning,
                TEXT("BOT HEALTH SEEK FAILED: %s could not reach %s"),
                *GetNameSafe(GetPawn()),
                *GetNameSafe(Pickup)
            );

            bSeekingHealthPickup = false;
            CurrentHealthPickup.Reset();

            MoveToRandomPatrolPoint();
        }

        return;
    }

    // Handle movement that was targeting an ammo pickup.
    if (bSeekingAmmoPickup)
    {
        AWeaponAmmoPickup* Pickup = CurrentAmmoPickup.Get();

        if (!IsValid(Pickup) ||
            Pickup->IsHidden() ||
            !Pickup->GetActorEnableCollision())
        {
            bSeekingAmmoPickup = false;
            CurrentAmmoPickup.Reset();
            MoveToRandomPatrolPoint();
            return;
        }

        if (Result.Code == EPathFollowingResult::Success)
        {
            ABotCharacter* Bot = Cast<ABotCharacter>(GetPawn());

            if (IsValid(Bot))
            {
                // Collect the pickup without requiring player input.
                Pickup->Interact(Bot);
            }

            bSeekingAmmoPickup = false;
            CurrentAmmoPickup.Reset();

            MoveToRandomPatrolPoint();
        }
        else
        {
            // Avoid repeatedly choosing the same unreachable pickup.
            FailedAmmoPickup = Pickup;

            UE_LOG(
                LogTemp,
                Warning,
                TEXT("BOT AMMO SEEK FAILED: %s could not reach %s"),
                *GetNameSafe(GetPawn()),
                *GetNameSafe(Pickup)
            );

            bSeekingAmmoPickup = false;
            CurrentAmmoPickup.Reset();

            MoveToRandomPatrolPoint();
        }

        return;
    }

    // If we still have a valid combat target, don't resume patrolling.
    if (IsValidCombatTarget(CurrentTarget.Get()))
    {
        return;
    }

    // Clear any stale target before returning to patrol.
    CurrentTarget.Reset();

    if (Result.Code == EPathFollowingResult::Success)
    {
        MoveToRandomPatrolPoint();
    }
}



void ABotAIController::MoveToRandomPatrolPoint()
{
    APawn* ControlledPawn = GetPawn();

    if (!ControlledPawn)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("PATROL DIAGNOSTIC: Controller has no pawn.")
        );
        return;
    }

    // Seek an active health pickup before continuing patrol.
    TrySeekHealthPickup();

    if (bSeekingHealthPickup)
    {
        return;
    }

    // Seek an ammo pickup if needed before continuing patrol.
    TrySeekAmmoPickup();

    if (bSeekingAmmoPickup)
    {
        return;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("PATROL DIAGNOSTIC: World not found.")
        );
        return;
    }

    UNavigationSystemV1* NavigationSystem =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);

    if (!NavigationSystem)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("PATROL DIAGNOSTIC: Navigation System not found.")
        );
        return;
    }

    const FVector ActorLocation = ControlledPawn->GetActorLocation();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("PATROL DIAGNOSTIC: Pawn=%s Location=%s"),
        *GetNameSafe(ControlledPawn),
        *ActorLocation.ToString()
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("PATROL DIAGNOSTIC: Navigation building or locked=%s"),
        UNavigationSystemV1::IsNavigationBeingBuiltOrLocked(World)
        ? TEXT("YES")
        : TEXT("NO")
    );

    const ACharacter* ControlledCharacter =
        Cast<ACharacter>(ControlledPawn);

    ANavigationData* SelectedNavData = nullptr;

    if (ControlledCharacter)
    {
        const FNavAgentProperties& AgentProperties =
            ControlledCharacter->GetNavAgentPropertiesRef();

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("PATROL DIAGNOSTIC: AgentRadius=%.1f AgentHeight=%.1f"),
            AgentProperties.AgentRadius,
            AgentProperties.AgentHeight
        );

        // Find navigation data compatible with this bot's agent properties.
        SelectedNavData = NavigationSystem->GetNavDataForProps(
            AgentProperties,
            ActorLocation,
            FVector(100.0f, 100.0f, 200.0f)
        );
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("PATROL DIAGNOSTIC: Selected NavData=%s"),
        *GetNameSafe(SelectedNavData)
    );

    // Test projection of the character's actor location.
    FNavLocation ProjectedActorLocation;

    const bool bActorProjected =
        NavigationSystem->ProjectPointToNavigation(
            ActorLocation,
            ProjectedActorLocation,
            FVector(100.0f, 100.0f, 200.0f),
            SelectedNavData
        );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("PATROL DIAGNOSTIC: Actor projection=%s ProjectedLocation=%s"),
        bActorProjected ? TEXT("SUCCESS") : TEXT("FAILED"),
        bActorProjected
        ? *ProjectedActorLocation.Location.ToString()
        : TEXT("None")
    );

    // Calculate an approximate floor position using the capsule.
    FVector FloorLocation = ActorLocation;

    if (ControlledCharacter)
    {
        if (const UCapsuleComponent* Capsule =
            ControlledCharacter->GetCapsuleComponent())
        {
            FloorLocation.Z -= Capsule->GetScaledCapsuleHalfHeight();
        }
    }

    // Test projection of the approximate floor location.
    FNavLocation ProjectedFloorLocation;

    const bool bFloorProjected =
        NavigationSystem->ProjectPointToNavigation(
            FloorLocation,
            ProjectedFloorLocation,
            FVector(100.0f, 100.0f, 200.0f),
            SelectedNavData
        );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("PATROL DIAGNOSTIC: Floor location=%s Floor projection=%s ProjectedLocation=%s"),
        *FloorLocation.ToString(),
        bFloorProjected ? TEXT("SUCCESS") : TEXT("FAILED"),
        bFloorProjected
        ? *ProjectedFloorLocation.Location.ToString()
        : TEXT("None")
    );

    if (!SelectedNavData)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("PATROL DIAGNOSTIC: No compatible navigation data found for %s"),
            *GetNameSafe(ControlledPawn)
        );
        return;
    }

    // Find a reachable patrol destination.
    FNavLocation RandomLocation;

    const bool bFoundLocation =
        NavigationSystem->GetRandomReachablePointInRadius(
            ActorLocation,
            1500.0f,
            RandomLocation
        );

    if (!bFoundLocation)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Bot patrol failed: No reachable destination for %s"),
            *GetNameSafe(ControlledPawn)
        );
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("PATROL DIAGNOSTIC: Random destination=%s"),
        *RandomLocation.Location.ToString()
    );

    MoveToLocation(RandomLocation.Location, 100.0f);
}

void ABotAIController::TrySeekHealthPickup()
{
    APawn* ControlledPawn = GetPawn();

    if (!IsValid(ControlledPawn) ||
        bIsRetreating ||
        bSeekingHealthPickup ||
        CurrentTarget.IsValid())
    {
        return;
    }

    ABaseCharacter* BotCharacter = Cast<ABaseCharacter>(ControlledPawn);

    if (!IsValid(BotCharacter))
    {
        return;
    }

    UHealthComponent* Health = BotCharacter->GetHealthComponent();

    if (!IsValid(Health) || !Health->IsAlive())
    {
        return;
    }

    const float MaxHealth = Health->GetMaxHealth();

    if (MaxHealth <= 0.0f ||
        Health->GetCurrentHealth() >= MaxHealth * 0.75f)
    {
        return;
    }

    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return;
    }

    UNavigationSystemV1* NavigationSystem =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);

    if (!IsValid(NavigationSystem))
    {
        return;
    }

    const ACharacter* ControlledCharacter =
        Cast<ACharacter>(ControlledPawn);

    if (!ControlledCharacter)
    {
        return;
    }

    const FNavAgentProperties& AgentProperties =
        ControlledCharacter->GetNavAgentPropertiesRef();

    ANavigationData* NavData = NavigationSystem->GetNavDataForProps(
        AgentProperties,
        ControlledPawn->GetActorLocation(),
        FVector(100.0f, 100.0f, 200.0f)
    );

    if (!NavData)
    {
        return;
    }

    AHealthPickup* BestPickup = nullptr;
    FVector BestLocation = FVector::ZeroVector;

    float BestDistanceSquared = TNumericLimits<float>::Max();

    for (TActorIterator<AHealthPickup> It(World); It; ++It)
    {
        AHealthPickup* Pickup = *It;

        if (!IsValid(Pickup) ||
            Pickup == FailedHealthPickup.Get() ||
            Pickup->IsHidden() ||
            !Pickup->GetActorEnableCollision())
        {
            continue;
        }

        FNavLocation ProjectedPickupLocation;

        const bool bProjected = NavigationSystem->ProjectPointToNavigation(
            Pickup->GetActorLocation(),
            ProjectedPickupLocation,
            FVector(150.0f, 150.0f, 250.0f),
            NavData
        );

        if (!bProjected)
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared(
            ControlledPawn->GetActorLocation(),
            ProjectedPickupLocation.Location
        );

        if (DistanceSquared < BestDistanceSquared)
        {
            BestDistanceSquared = DistanceSquared;
            BestPickup = Pickup;
            BestLocation = ProjectedPickupLocation.Location;
        }
    }

    if (!IsValid(BestPickup))
    {
        return;
    }

    CurrentHealthPickup = BestPickup;
    bSeekingHealthPickup = true;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("BOT SEEKING HEALTH: %s -> %s"),
        *GetNameSafe(ControlledPawn),
        *GetNameSafe(BestPickup)
    );

    const EPathFollowingRequestResult::Type MoveResult =
        MoveToLocation(BestLocation, 100.0f);

    if (MoveResult != EPathFollowingRequestResult::RequestSuccessful &&
        MoveResult != EPathFollowingRequestResult::AlreadyAtGoal)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("BOT HEALTH SEEK FAILED: %s"),
            *GetNameSafe(ControlledPawn)
        );

        CurrentHealthPickup.Reset();
        bSeekingHealthPickup = false;
    }
}


void ABotAIController::TrySeekAmmoPickup()
{
    APawn* ControlledPawn = GetPawn();

    if (!IsValid(ControlledPawn) ||
        bIsRetreating ||
        bSeekingHealthPickup ||
        bSeekingAmmoPickup ||
        CurrentTarget.IsValid())
    {
        return;
    }

    ABotCharacter* Bot = Cast<ABotCharacter>(ControlledPawn);

    if (!IsValid(Bot) || !IsValid(Bot->WeaponInventory))
    {
        return;
    }

    UWeaponInventoryComponent* Inventory = Bot->WeaponInventory;

    ASecondaryWeapon* SecondaryWeapon =
        Cast<ASecondaryWeapon>(Inventory->GetSecondaryWeapon());

    // Seek a pickup if the bot has no secondary weapon,
    // or if its secondary ammunition is below half capacity.
    if (IsValid(SecondaryWeapon))
    {
        const int32 MaxAmmo = SecondaryWeapon->GetMaxAmmo();
        const int32 CurrentAmmo = SecondaryWeapon->GetCurrentAmmo();

        if (MaxAmmo > 0 && CurrentAmmo >= MaxAmmo / 2)
        {
            return;
        }
    }

    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        return;
    }

    UNavigationSystemV1* NavigationSystem =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);

    if (!IsValid(NavigationSystem))
    {
        return;
    }

    ACharacter* ControlledCharacter =
        Cast<ACharacter>(ControlledPawn);

    if (!ControlledCharacter)
    {
        return;
    }

    const FNavAgentProperties& AgentProperties =
        ControlledCharacter->GetNavAgentPropertiesRef();

    ANavigationData* NavData = NavigationSystem->GetNavDataForProps(
        AgentProperties,
        ControlledPawn->GetActorLocation(),
        FVector(100.0f, 100.0f, 200.0f)
    );

    if (!NavData)
    {
        return;
    }

    AWeaponAmmoPickup* BestPickup = nullptr;
    FVector BestLocation = FVector::ZeroVector;
    float BestDistanceSquared = TNumericLimits<float>::Max();

    for (TActorIterator<AWeaponAmmoPickup> It(World); It; ++It)
    {
        AWeaponAmmoPickup* Pickup = *It;

        if (!IsValid(Pickup) ||
            Pickup == FailedAmmoPickup.Get() ||
            Pickup->IsHidden() ||
            !Pickup->GetActorEnableCollision())
        {
            continue;
        }

        FNavLocation ProjectedPickupLocation;

        const bool bProjected =
            NavigationSystem->ProjectPointToNavigation(
                Pickup->GetActorLocation(),
                ProjectedPickupLocation,
                FVector(150.0f, 150.0f, 250.0f),
                NavData
            );

        if (!bProjected)
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared(
            ControlledPawn->GetActorLocation(),
            ProjectedPickupLocation.Location
        );

        if (DistanceSquared < BestDistanceSquared)
        {
            BestDistanceSquared = DistanceSquared;
            BestPickup = Pickup;
            BestLocation = ProjectedPickupLocation.Location;
        }
    }

    if (!IsValid(BestPickup))
    {
        return;
    }

    CurrentAmmoPickup = BestPickup;
    bSeekingAmmoPickup = true;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("BOT SEEKING AMMO: %s -> %s"),
        *GetNameSafe(ControlledPawn),
        *GetNameSafe(BestPickup)
    );

    const EPathFollowingRequestResult::Type MoveResult =
        MoveToLocation(BestLocation, 100.0f);

    if (MoveResult != EPathFollowingRequestResult::RequestSuccessful &&
        MoveResult != EPathFollowingRequestResult::AlreadyAtGoal)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("BOT AMMO SEEK FAILED: %s"),
            *GetNameSafe(ControlledPawn)
        );

        CurrentAmmoPickup.Reset();
        bSeekingAmmoPickup = false;
    }
}




void ABotAIController::HandleTargetPerceptionUpdated(
    AActor* Actor,
    FAIStimulus Stimulus
)
{
    if (!IsValidCombatTarget(Actor))
    {
        return;
    }

    // Retreating bots must not restart pursuit or combat.
    if (bIsRetreating)
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        if (!IsValidCombatTarget(CurrentTarget.Get()))
        {
            // Combat takes priority over health seeking.
            bSeekingHealthPickup = false;
            CurrentHealthPickup.Reset();

            CurrentTarget = Actor;
            PursueCurrentTarget();
            StartFiringAtTarget();

            UE_LOG(
                LogTemp,
                Warning,
                TEXT("BOT TARGET SELECTED: %s is pursuing %s"),
                *GetNameSafe(GetPawn()),
                *GetNameSafe(Actor)
            );
        }
    }
    else if (CurrentTarget.Get() == Actor)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("BOT TARGET LOST: %s lost sight of %s"),
            *GetNameSafe(GetPawn()),
            *GetNameSafe(Actor)
        );

        StopFiringAtTarget();
        CurrentTarget.Reset();

        // Do not cancel retreat movement when a target is lost.
        if (bIsRetreating)
        {
            return;
        }

        StopMovement();
        MoveToRandomPatrolPoint();
    }
}



bool ABotAIController::IsValidCombatTarget(AActor* Actor) const
{
    if (!IsValid(Actor) || Actor == GetPawn())
    {
        return false;
    }

    return Actor->IsA(APlayerCharacter::StaticClass())
        || Actor->IsA(ABotCharacter::StaticClass());
}


bool ABotAIController::IsLowOnHealth() const
{
    const ABaseCharacter* ControlledCharacter =
        Cast<ABaseCharacter>(GetPawn());

    if (!IsValid(ControlledCharacter))
    {
        return false;
    }

    const UHealthComponent* Health =
        ControlledCharacter->GetHealthComponent();

    if (!IsValid(Health))
    {
        return false;
    }

    const float MaxHealth = Health->GetMaxHealth();

    if (MaxHealth <= 0.0f)
    {
        return false;
    }

    const float HealthPercentage =
        Health->GetCurrentHealth() / MaxHealth;

    return HealthPercentage < LowHealthThreshold;
}




void ABotAIController::RetreatFromCombat()
{
    APawn* ControlledPawn = GetPawn();
    UWorld* World = GetWorld();
    AActor* Target = CurrentTarget.Get();

    const float Time = World ? World->GetTimeSeconds() : -1.0f;

    if (!IsValid(ControlledPawn))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("RETREAT DIAGNOSTIC: Time=%.3f | No valid controlled pawn"),
            Time
        );
        return;
    }

    const FVector CurrentLocation = ControlledPawn->GetActorLocation();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("RETREAT ATTEMPT: Time=%.3f | Pawn=%s | Location=%s | Target=%s | TargetLocation=%s | Retreating=%s | RetryUsed=%s"),
        Time,
        *GetNameSafe(ControlledPawn),
        *CurrentLocation.ToString(),
        *GetNameSafe(Target),
        IsValid(Target)
        ? *Target->GetActorLocation().ToString()
        : TEXT("None"),
        bIsRetreating ? TEXT("YES") : TEXT("NO"),
        bRetreatRetryUsed ? TEXT("YES") : TEXT("NO")
    );

    if (!IsValid(World))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("RETREAT FAILURE: World is invalid for %s"),
            *GetNameSafe(ControlledPawn)
        );
        return;
    }

    UNavigationSystemV1* NavigationSystem =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);

    if (!IsValid(NavigationSystem))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("RETREAT FAILURE: Navigation system unavailable for %s"),
            *GetNameSafe(ControlledPawn)
        );
        return;
    }

    FVector AwayDirection = FVector::ZeroVector;

    if (IsValidCombatTarget(Target))
    {
        AwayDirection =
            (CurrentLocation - Target->GetActorLocation()).GetSafeNormal2D();
    }

    if (AwayDirection.IsNearlyZero())
    {
        AwayDirection = FMath::VRand();
        AwayDirection.Z = 0.0f;
        AwayDirection.Normalize();
    }

    const ACharacter* ControlledCharacter =
        Cast<ACharacter>(ControlledPawn);

    ANavigationData* SelectedNavData = nullptr;

    if (ControlledCharacter)
    {
        SelectedNavData = NavigationSystem->GetNavDataForProps(
            ControlledCharacter->GetNavAgentPropertiesRef(),
            CurrentLocation,
            FVector(100.0f, 100.0f, 200.0f)
        );
    }

    if (!SelectedNavData)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("RETREAT FAILURE: No compatible nav data | Pawn=%s | Location=%s"),
            *GetNameSafe(ControlledPawn),
            *CurrentLocation.ToString()
        );
        return;
    }


    const float RetreatDistances[] = {
        RetreatDistance,
        RetreatDistance * 0.75f,
        RetreatDistance * 0.5f
    };

    const float RetreatYawOffsets[] = {
        0.0f,
        45.0f,
        -45.0f,
        90.0f,
        -90.0f
    };

    int32 ProjectionAttempts = 0;
    int32 SuccessfulProjections = 0;
    int32 MoveRequestsFailed = 0;

    for (const float YawOffset : RetreatYawOffsets)
    {
        const FVector RetreatDirection =
            AwayDirection.RotateAngleAxis(YawOffset, FVector::UpVector)
            .GetSafeNormal2D();

        for (const float Distance : RetreatDistances)
        {
            ++ProjectionAttempts;

            const FVector DesiredLocation =
                CurrentLocation + RetreatDirection * Distance;

            FNavLocation ProjectedLocation;

            const bool bProjected =
                NavigationSystem->ProjectPointToNavigation(
                    DesiredLocation,
                    ProjectedLocation,
                    FVector(500.0f, 500.0f, 300.0f),
                    SelectedNavData
                );

            UE_LOG(
                LogTemp,
                Warning,
                TEXT("RETREAT CANDIDATE: Time=%.3f | Pawn=%s | YawOffset=%.1f | Distance=%.1f | Desired=%s | Projected=%s | Result=%s"),
                World->GetTimeSeconds(),
                *GetNameSafe(ControlledPawn),
                YawOffset,
                Distance,
                *DesiredLocation.ToString(),
                bProjected ? *ProjectedLocation.Location.ToString() : TEXT("None"),
                bProjected ? TEXT("SUCCESS") : TEXT("FAILED")
            );

            if (!bProjected)
            {
                continue;
            }

            ++SuccessfulProjections;

            // Only accept destinations that do not bring the bot closer
            // to its current combat target.
            if (IsValidCombatTarget(Target))
            {
                const float CurrentTargetDistance =
                    FVector::Dist2D(
                        CurrentLocation,
                        Target->GetActorLocation()
                    );

                const float ProjectedTargetDistance =
                    FVector::Dist2D(
                        ProjectedLocation.Location,
                        Target->GetActorLocation()
                    );

                if (ProjectedTargetDistance <= CurrentTargetDistance)
                {
                    UE_LOG(
                        LogTemp,
                        Warning,
                        TEXT("RETREAT CANDIDATE REJECTED: Time=%.3f | Pawn=%s | YawOffset=%.1f | Destination does not increase target distance"),
                        World->GetTimeSeconds(),
                        *GetNameSafe(ControlledPawn),
                        YawOffset
                    );

                    continue;
                }
            }

            const EPathFollowingRequestResult::Type MoveResult =
                MoveToLocation(ProjectedLocation.Location, 100.0f);

            UE_LOG(
                LogTemp,
                Warning,
                TEXT("RETREAT MOVE REQUEST: Time=%.3f | Pawn=%s | YawOffset=%.1f | Result=%d | Destination=%s"),
                World->GetTimeSeconds(),
                *GetNameSafe(ControlledPawn),
                YawOffset,
                static_cast<int32>(MoveResult),
                *ProjectedLocation.Location.ToString()
            );

            if (MoveResult != EPathFollowingRequestResult::Failed)
            {
                UE_LOG(
                    LogTemp,
                    Warning,
                    TEXT("RETREAT DESTINATION ACCEPTED: Time=%.3f | Pawn=%s | YawOffset=%.1f | Destination=%s"),
                    World->GetTimeSeconds(),
                    *GetNameSafe(ControlledPawn),
                    YawOffset,
                    *ProjectedLocation.Location.ToString()
                );

                return;
            }

            ++MoveRequestsFailed;
        }
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("RETREAT FAILURE: No destination accepted | Time=%.3f | Pawn=%s | Location=%s | ProjectionAttempts=%d | SuccessfulProjections=%d | FailedMoveRequests=%d"),
        World->GetTimeSeconds(),
        *GetNameSafe(ControlledPawn),
        *ControlledPawn->GetActorLocation().ToString(),
        ProjectionAttempts,
        SuccessfulProjections,
        MoveRequestsFailed


    );

    bRetreatFailed = true;
    bIsRetreating = false;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("BOT RETREAT FAILED: %s will defend itself"),
        *GetNameSafe(ControlledPawn)
    );

}



void ABotAIController::StopRetreating()
{
    if (!bIsRetreating)
    {
        return;
    }

    bIsRetreating = false;
    bRetreatFailed = false;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("BOT RETREAT ENDED: %s health recovered above 25%%"),
        *GetNameSafe(GetPawn())
    );

    if (IsValidCombatTarget(CurrentTarget.Get()))
    {
        PursueCurrentTarget();
    }
    else
    {
        MoveToRandomPatrolPoint();
    }
}




void ABotAIController::PursueCurrentTarget()
{
    AActor* Target = CurrentTarget.Get();

    if (!IsValidCombatTarget(Target))
    {
        CurrentTarget.Reset();
        return;
    }

    MoveToActor(Target, 300.0f, true);
}

void ABotAIController::StartFiringAtTarget()
{
    if (!IsValidCombatTarget(CurrentTarget.Get()))
    {
        return;
    }

    GetWorldTimerManager().SetTimer(
        CombatFireTimerHandle,
        this,
        &ABotAIController::FireAtCurrentTarget,
        CombatFireIntervalSeconds,
        true,
        0.0f
    );
}

void ABotAIController::StopFiringAtTarget()
{
    GetWorldTimerManager().ClearTimer(CombatFireTimerHandle);
}



void ABotAIController::FireAtCurrentTarget()
{
    AActor* Target = CurrentTarget.Get();

    if (!IsValidCombatTarget(Target))
    {
        StopFiringAtTarget();
        return;
    }

    // While retreating, keep checking health without firing.
    if (bIsRetreating)
    {
        if (IsLowOnHealth())
        {
            return;
        }

        // Health has recovered above the retreat threshold.
        StopRetreating();
    }
    else if (IsLowOnHealth() && !bRetreatFailed)
    {
        bIsRetreating = true;

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("BOT RETREAT STARTED: %s health is below 25%%"),
            *GetNameSafe(GetPawn())
        );

        RetreatFromCombat();
        return;
    }

    ABotCharacter* Bot = Cast<ABotCharacter>(GetPawn());

    if (!IsValid(Bot) || !IsValid(Bot->WeaponInventory))
    {
        return;
    }

    // Use the secondary weapon while it has ammunition.
    // Otherwise, fall back to the default weapon.
    ASecondaryWeapon* SecondaryWeapon =
        Cast<ASecondaryWeapon>(
            Bot->WeaponInventory->GetSecondaryWeapon()
        );

    if (IsValid(SecondaryWeapon) &&
        SecondaryWeapon->GetCurrentAmmo() > 0)
    {
        Bot->WeaponInventory->EquipSecondaryWeapon();
    }
    else
    {
        Bot->WeaponInventory->EquipDefaultWeapon();
    }

    ABaseWeapon* Weapon =
        Bot->WeaponInventory->GetEquippedWeapon();

    if (!IsValid(Weapon))
    {
        return;
    }

    const bool bFired = Weapon->FireAtTarget(Target);

    if (bFired)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("BOT FIRED: %s fired at %s"),
            *GetNameSafe(Bot),
            *GetNameSafe(Target)
        );
    }
}


