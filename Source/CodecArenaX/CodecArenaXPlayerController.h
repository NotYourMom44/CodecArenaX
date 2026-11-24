
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CodecArenaXGameState.h"
#include "CodecArenaXPlayerController.generated.h"

class UInputAction;
class UUserWidget;

UCLASS()
class CODECARENAX_API ACodecArenaXPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual ~ACodecArenaXPlayerController() override;

    UFUNCTION(BlueprintCallable, Category = "Pause")
    void ResumeGame();

    UFUNCTION(BlueprintCallable, Category = "Match")
    void RestartMatch();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    // State Pattern classes.
    class FControllerState;
    class FPlayingState;
    class FPausedState;
    class FMatchOverState;

    TSharedPtr<FControllerState> CurrentState;

    void TransitionToState(TSharedPtr<FControllerState> NewState);

    // Input handling.
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> ShowLeaderboardAction = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> PauseAction = nullptr;

    // Widget classes.
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> LeaderboardWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> MatchOverWidgetClass;

    // Widget instances.
    UPROPERTY()
    TObjectPtr<UUserWidget> LeaderboardWidget = nullptr;

    UPROPERTY()
    TObjectPtr<UUserWidget> PauseMenuWidget = nullptr;

    UPROPERTY()
    TObjectPtr<UUserWidget> MatchOverWidget = nullptr;

    // State-dispatched input.
    void ShowLeaderboard();
    void HideLeaderboard();
    void TogglePause();

    // State transition actions.
    void OpenPauseMenu();
    void ClosePauseMenu();
    void OpenMatchOverScreen();

    UFUNCTION()
    void HandleMatchStateChanged(ECodecArenaMatchState NewMatchState);
};
