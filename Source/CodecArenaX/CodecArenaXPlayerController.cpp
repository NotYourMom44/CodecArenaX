
#include "CodecArenaXPlayerController.h"

#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void ACodecArenaXPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }

    bShowMouseCursor = false;

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);

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

void ACodecArenaXPlayerController::ShowLeaderboard()
{
    if (LeaderboardWidget &&
        !UGameplayStatics::IsGamePaused(GetWorld()))
    {
        LeaderboardWidget->SetVisibility(
            ESlateVisibility::Visible
        );
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
    if (!PauseMenuWidget)
    {
        return;
    }

    ACodecArenaXGameState* MatchGameState =
        GetWorld()->GetGameState<ACodecArenaXGameState>();

    if (MatchGameState &&
        MatchGameState->GetMatchState() ==
        ECodecArenaMatchState::MatchOver)
    {
        return;
    }

    if (UGameplayStatics::IsGamePaused(GetWorld()))
    {
        SetPause(false);

        PauseMenuWidget->SetVisibility(
            ESlateVisibility::Collapsed
        );

        bShowMouseCursor = false;

        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
    }
    else
    {
        HideLeaderboard();

        PauseMenuWidget->SetVisibility(
            ESlateVisibility::Visible
        );

        SetPause(true);

        bShowMouseCursor = true;

        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(
            EMouseLockMode::DoNotLock
        );
        InputMode.SetHideCursorDuringCapture(false);

        SetInputMode(InputMode);
    }
}

void ACodecArenaXPlayerController::HandleMatchStateChanged(
    ECodecArenaMatchState NewMatchState
)
{
    if (NewMatchState != ECodecArenaMatchState::MatchOver)
    {
        return;
    }

    HideLeaderboard();

    if (PauseMenuWidget)
    {
        PauseMenuWidget->SetVisibility(
            ESlateVisibility::Collapsed
        );
    }

    if (!MatchOverWidget)
    {
        return;
    }

    MatchOverWidget->SetVisibility(
        ESlateVisibility::Visible
    );

    SetPause(true);

    bShowMouseCursor = true;

    FInputModeGameAndUI InputMode;
    InputMode.SetWidgetToFocus(MatchOverWidget->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(
        EMouseLockMode::DoNotLock
    );
    InputMode.SetHideCursorDuringCapture(false);

    SetInputMode(InputMode);
}

void ACodecArenaXPlayerController::ResumeGame()
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

void ACodecArenaXPlayerController::RestartMatch()
{
    SetPause(false);

    bShowMouseCursor = false;

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);

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
