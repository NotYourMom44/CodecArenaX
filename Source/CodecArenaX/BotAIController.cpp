#include "BotAIController.h"

#include "WeaponInventoryComponent.h"
#include "BaseWeapon.h"
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


void ABotAIController::HandleTargetPerceptionUpdated(
    AActor* Actor,
    FAIStimulus Stimulus
)
{
    if (!IsValidCombatTarget(Actor))
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        if (!IsValidCombatTarget(CurrentTarget.Get()))
        {
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

    ABotCharacter* Bot = Cast<ABotCharacter>(GetPawn());

    if (!IsValid(Bot) || !IsValid(Bot->WeaponInventory))
    {
        return;
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
