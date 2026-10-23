
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Engine/TimerHandle.h"
#include "Perception/AIPerceptionTypes.h"
#include "BotAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class CODECARENAX_API ABotAIController : public AAIController
{
    GENERATED_BODY()

public:
    ABotAIController();

protected:
    virtual void OnPossess(APawn* InPawn) override;

    virtual void OnMoveCompleted(
        FAIRequestID RequestID,
        const FPathFollowingResult& Result
    ) override;

private:
    void MoveToRandomPatrolPoint();

    // Read-only snapshot of navigation registration state.
    void LogNavigationRegistrationState(const TCHAR* Context);

    // Timer callback: logs the snapshot once, shortly after possession.
    void LogDelayedNavigationState();

    FTimerHandle NavDiagnosticTimerHandle;

    static constexpr float NavDiagnosticDelaySeconds = 2.0f;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    TObjectPtr<UAIPerceptionComponent> AIPerception;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    TWeakObjectPtr<AActor> CurrentTarget;

    // Health-pickup seeking while patrolling.
    TWeakObjectPtr<class AHealthPickup> CurrentHealthPickup;
    TWeakObjectPtr<class AHealthPickup> FailedHealthPickup;

    bool bSeekingHealthPickup = false;

    void TrySeekHealthPickup();

    bool IsValidCombatTarget(AActor* Actor) const;

    void PursueCurrentTarget();

    void StartFiringAtTarget();
    void StopFiringAtTarget();

    void FireAtCurrentTarget();

    FTimerHandle CombatFireTimerHandle;

    static constexpr float CombatFireIntervalSeconds = 1.0f;

    // Low-health retreat behaviour.
    bool IsLowOnHealth() const;

    bool bRetreatRetryUsed = false;
    bool bRetreatFailed = false;

    void RetreatFromCombat();
    void StopRetreating();

    bool bIsRetreating = false;

    static constexpr float LowHealthThreshold = 0.25f;
    static constexpr float RetreatDistance = 1500.0f;

    UFUNCTION()
    void HandleTargetPerceptionUpdated(
        AActor* Actor,
        FAIStimulus Stimulus
    );
};
