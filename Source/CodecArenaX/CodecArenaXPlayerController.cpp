
#include "CodecArenaXPlayerController.h"

#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Blueprint/UserWidget.h"

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
}

void ACodecArenaXPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

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
}

void ACodecArenaXPlayerController::ShowLeaderboard()
{
    if (LeaderboardWidget)
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
