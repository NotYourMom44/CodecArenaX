
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

    
    // State Pattern
    

    class FBotAIState;
    class FPatrolState;
    class FCombatState;
    class FSeekingHealthState;
    class FSeekingAmmoState;
    class FRetreatingState;

    TSharedPtr<FBotAIState> CurrentState;

    void TransitionToState(TSharedPtr<FBotAIState> NewState);

    
    // Patrol and navigation diagnostics
    

    void MoveToRandomPatrolPoint();

    void LogNavigationRegistrationState(const TCHAR* Context);
    void LogDelayedNavigationState();

    FTimerHandle NavDiagnosticTimerHandle;

    static constexpr float NavDiagnosticDelaySeconds = 2.0f;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    TObjectPtr<UAIPerceptionComponent> AIPerception;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    
    // Combat target
    

    TWeakObjectPtr<AActor> CurrentTarget;

    bool IsValidCombatTarget(AActor* Actor) const;

    void PursueCurrentTarget();

    void StartFiringAtTarget();
    void StopFiringAtTarget();

    void FireAtCurrentTarget();

    FTimerHandle CombatFireTimerHandle;

    static constexpr float CombatFireIntervalSeconds = 1.0f;

    
    // Health pickup seeking
   

    TWeakObjectPtr<class AHealthPickup> CurrentHealthPickup;
    TWeakObjectPtr<class AHealthPickup> FailedHealthPickup;

    bool bSeekingHealthPickup = false;

    void TrySeekHealthPickup();

    
    // Ammo pickup seeking
    

    TWeakObjectPtr<class AWeaponAmmoPickup> CurrentAmmoPickup;
    TWeakObjectPtr<class AWeaponAmmoPickup> FailedAmmoPickup;

    bool bSeekingAmmoPickup = false;

    void TrySeekAmmoPickup();

    
    // Low-health retreat behaviour
    

    bool IsLowOnHealth() const;

    bool bRetreatRetryUsed = false;
    bool bRetreatFailed = false;

    void RetreatFromCombat();
    void StopRetreating();

    bool bIsRetreating = false;
    bool bIssuingRetreatMove = false;

    static constexpr float LowHealthThreshold = 0.25f;
    static constexpr float RetreatDistance = 1500.0f;

    static constexpr float RetreatArenaBoundary = 3400.0f;

    
    // Perception
    

    UFUNCTION()
    void HandleTargetPerceptionUpdated(
        AActor* Actor,
        FAIStimulus Stimulus
    );
};
