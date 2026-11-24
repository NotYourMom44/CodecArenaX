
#include "CodecArenaXPlayerController.h"

#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

// ------------------------------------------------------------
// STATE PATTERN: Base state
// ------------------------------------------------------------

class ACodecArenaXPlayerController::FControllerState
{
public:
    explicit FControllerState(ACodecArenaXPlayerController& InController)
        : Controller(InController)
    {}

    virtual ~FControllerState() = default;

    virtual void Enter() {}
    virtual void Exit() {}

    virtual void TogglePause() {}
    virtual void ShowLeaderboard() {}
    virtual void HideLeaderboard() {}

protected:
    ACodecArenaXPlayerController& Controller;
};


// ------------------------------------------------------------
// Concrete state declarations
// ------------------------------------------------------------

class ACodecArenaXPlayerController::FPlayingState
    : public ACodecArenaXPlayerController::FControllerState
{
public:
    using FControllerState::FControllerState;

    void Enter() override;
    void TogglePause() override;
    void ShowLeaderboard() override;
    void HideLeaderboard() override;
};


class ACodecArenaXPlayerController::FPausedState
    : public ACodecArenaXPlayerController::FControllerState
{
public:
    using FControllerState::FControllerState;

    void Enter() override;
    void TogglePause() override;
};


class ACodecArenaXPlayerController::FMatchOverState
    : public ACodecArenaXPlayerController::FControllerState
{
public:
    using FControllerState::FControllerState;

    void Enter() override;
};


// ------------------------------------------------------------
// Playing state implementation
// ------------------------------------------------------------

void ACodecArenaXPlayerController::FPlayingState::Enter()
{
    Controller.ClosePauseMenu();
}

void ACodecArenaXPlayerController::FPlayingState::TogglePause()
{
    TSharedPtr<FControllerState> NewState =
        MakeShared<FPausedState>(Controller);

    Controller.TransitionToState(MoveTemp(NewState));
}

void ACodecArenaXPlayerController::FPlayingState::ShowLeaderboard()
{
    if (Controller.LeaderboardWidget)
    {
        Controller.LeaderboardWidget->SetVisibility(
            ESlateVisibility::Visible
        );
    }
}

void ACodecArenaXPlayerController::FPlayingState::HideLeaderboard()
{
    Controller.HideLeaderboard();
}


// ------------------------------------------------------------
// Paused state implementation
// ------------------------------------------------------------

void ACodecArenaXPlayerController::FPausedState::Enter()
{
    Controller.OpenPauseMenu();
}

void ACodecArenaXPlayerController::FPausedState::TogglePause()
{
    TSharedPtr<FControllerState> NewState =
        MakeShared<FPlayingState>(Controller);

    Controller.TransitionToState(MoveTemp(NewState));
}


// ------------------------------------------------------------
// Match Over state implementation
// ------------------------------------------------------------

void ACodecArenaXPlayerController::FMatchOverState::Enter()
{
    Controller.OpenMatchOverScreen();
}


// ------------------------------------------------------------
// State transition management
// ------------------------------------------------------------

ACodecArenaXPlayerController::~ACodecArenaXPlayerController() = default;

void ACodecArenaXPlayerController::TransitionToState(
    TSharedPtr<FControllerState> NewState
)
{
    if (!NewState.IsValid())
    {
        return;
    }

    if (CurrentState.IsValid())
    {
        CurrentState->Exit();
    }

    CurrentState = MoveTemp(NewState);
    CurrentState->Enter();
}


// ------------------------------------------------------------
// Begin Play
// ------------------------------------------------------------

void ACodecArenaXPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }

    if (LeaderboardWidgetClass)
    {
        LeaderboardWidget = CreateWidget<UUserWidget>(
            this,
            LeaderboardWidgetClass
        );

        if (LeaderboardWidget)
        {
            LeaderboardWidget->AddToViewport();
            LeaderboardWidget->SetVisibility(
                ESlateVisibility::Collapsed
            );
        }
    }

    if (PauseMenuWidgetClass)
    {
        PauseMenuWidget = CreateWidget<UUserWidget>(
            this,
            PauseMenuWidgetClass
        );

        if (PauseMenuWidget)
        {
            PauseMenuWidget->AddToViewport();
            PauseMenuWidget->SetVisibility(
                ESlateVisibility::Collapsed
            );
        }
    }

    if (MatchOverWidgetClass)
    {
        MatchOverWidget = CreateWidget<UUserWidget>(
            this,
            MatchOverWidgetClass
        );

        if (MatchOverWidget)
        {
            MatchOverWidget->AddToViewport();
            MatchOverWidget->SetVisibility(
                ESlateVisibility::Collapsed
            );
        }
    }

    // Enter the initial Playing state.
    TSharedPtr<FControllerState> InitialState =
        MakeShared<FPlayingState>(*this);

    TransitionToState(MoveTemp(InitialState));

    ACodecArenaXGameState* MatchGameState =
        GetWorld()->GetGameState<ACodecArenaXGameState>();

    if (MatchGameState)
    {
        MatchGameState->OnMatchStateChanged.AddDynamic(
            this,
            &ACodecArenaXPlayerController::HandleMatchStateChanged
        );

        if (MatchGameState->GetMatchState() ==
            ECodecArenaMatchState::MatchOver)
        {
            HandleMatchStateChanged(
                ECodecArenaMatchState::MatchOver
            );
        }
    }
}


// ------------------------------------------------------------
// Input setup
// ------------------------------------------------------------

void ACodecArenaXPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (!InputComponent)
    {
        return;
    }

    UEnhancedInputComponent* EnhancedInputComponent =
        Cast<UEnhancedInputComponent>(InputComponent);

    if (!EnhancedInputComponent)
    {
        return;
    }

    if (ShowLeaderboardAction)
    {
        EnhancedInputComponent->BindAction(
            ShowLeaderboardAction,
            ETriggerEvent::Started,
            this,
            &ACodecArenaXPlayerController::ShowLeaderboard
        );

        EnhancedInputComponent->BindAction(
            ShowLeaderboardAction,
            ETriggerEvent::Completed,
            this,
            &ACodecArenaXPlayerController::HideLeaderboard
        );
    }

    if (PauseAction)
    {
        EnhancedInputComponent->BindAction(
            PauseAction,
            ETriggerEvent::Started,
            this,
            &ACodecArenaXPlayerController::TogglePause
        );
    }
}


// ------------------------------------------------------------
// State-dispatched input
// ------------------------------------------------------------

void ACodecArenaXPlayerController::ShowLeaderboard()
{
    if (CurrentState.IsValid())
    {
        CurrentState->ShowLeaderboard();
    }
}

void ACodecArenaXPlayerController::HideLeaderboard()
{
    if (LeaderboardWidget)
    {
        LeaderboardWidget->SetVisibility(
            ESlateVisibility::Collapsed
        );
    }
}

void ACodecArenaXPlayerController::TogglePause()
{
    if (CurrentState.IsValid())
    {
        CurrentState->TogglePause();
    }
}


// ------------------------------------------------------------
// Pause menu actions
// ------------------------------------------------------------

void ACodecArenaXPlayerController::OpenPauseMenu()
{
    HideLeaderboard();

    if (PauseMenuWidget)
    {
        PauseMenuWidget->SetVisibility(
            ESlateVisibility::Visible
        );
    }

    SetPause(true);

    bShowMouseCursor = true;

    FInputModeGameAndUI InputMode;

    if (PauseMenuWidget)
    {
        InputMode.SetWidgetToFocus(
            PauseMenuWidget->TakeWidget()
        );
    }

    InputMode.SetLockMouseToViewportBehavior(
        EMouseLockMode::DoNotLock
    );

    InputMode.SetHideCursorDuringCapture(false);

    SetInputMode(InputMode);
}

void ACodecArenaXPlayerController::ClosePauseMenu()
{
    SetPause(false);

    if (PauseMenuWidget)
    {
        PauseMenuWidget->SetVisibility(
            ESlateVisibility::Collapsed
        );
    }

    if (MatchOverWidget)
    {
        MatchOverWidget->SetVisibility(
            ESlateVisibility::Collapsed
        );
    }

    bShowMouseCursor = false;

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
}


// ------------------------------------------------------------
// Match Over screen action
// ------------------------------------------------------------

void ACodecArenaXPlayerController::OpenMatchOverScreen()
{
    HideLeaderboard();

    if (PauseMenuWidget)
    {
        PauseMenuWidget->SetVisibility(
            ESlateVisibility::Collapsed
        );
    }

    if (MatchOverWidget)
    {
        MatchOverWidget->SetVisibility(
            ESlateVisibility::Visible
        );
    }

    SetPause(true);

    bShowMouseCursor = true;

    FInputModeGameAndUI InputMode;

    if (MatchOverWidget)
    {
        InputMode.SetWidgetToFocus(
            MatchOverWidget->TakeWidget()
        );
    }

    InputMode.SetLockMouseToViewportBehavior(
        EMouseLockMode::DoNotLock
    );

    InputMode.SetHideCursorDuringCapture(false);

    SetInputMode(InputMode);
}


// ------------------------------------------------------------
// Game State notification
// ------------------------------------------------------------

void ACodecArenaXPlayerController::HandleMatchStateChanged(
    ECodecArenaMatchState NewMatchState
)
{
    if (NewMatchState == ECodecArenaMatchState::MatchOver)
    {
        TSharedPtr<FControllerState> NewState =
            MakeShared<FMatchOverState>(*this);

        TransitionToState(MoveTemp(NewState));
    }
}


// ------------------------------------------------------------
// Blueprint-callable actions
// ------------------------------------------------------------

void ACodecArenaXPlayerController::ResumeGame()
{
    TSharedPtr<FControllerState> NewState =
        MakeShared<FPlayingState>(*this);

    TransitionToState(MoveTemp(NewState));
}

void ACodecArenaXPlayerController::RestartMatch()
{
    SetPause(false);

    TSharedPtr<FControllerState> NewState =
        MakeShared<FPlayingState>(*this);

    TransitionToState(MoveTemp(NewState));

    const FString CurrentLevelName =
        UGameplayStatics::GetCurrentLevelName(this, true);

    if (!CurrentLevelName.IsEmpty())
    {
        UGameplayStatics::OpenLevel(
            this,
            FName(*CurrentLevelName)
        );
    }
}
