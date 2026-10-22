#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Engine/TimerHandle.h"
#include "BotAIController.generated.h"

UCLASS()
class CODECARENAX_API ABotAIController : public AAIController
{
    GENERATED_BODY()

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
};