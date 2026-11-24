
#include "BotHealthBarWidget.h"

#include "HealthComponent.h"
#include "Components/ProgressBar.h"

void UBotHealthBarWidget::InitializeHealthBar(
    UHealthComponent* InHealthComponent
)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("HealthBar: Initializing for %s"),
        *GetNameSafe(InHealthComponent)
    );

    if (!InHealthComponent)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("HealthBar: Health component is NULL!")
        );
        return;
    }

    if (!HealthProgressBar)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("HealthBar: HealthProgressBar is NULL! Check the BindWidget name.")
        );
        return;
    }

    BoundHealthComponent = InHealthComponent;

    BoundHealthComponent->OnHealthChanged.AddDynamic(
        this,
        &UBotHealthBarWidget::HandleHealthChanged
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("HealthBar: Successfully bound to health component.")
    );

    HandleHealthChanged(
        BoundHealthComponent->GetCurrentHealth(),
        BoundHealthComponent->GetMaxHealth()
    );
}

void UBotHealthBarWidget::HandleHealthChanged(
    float CurrentHealth,
    float MaxHealth
)
{
    if (!HealthProgressBar || MaxHealth <= 0.0f)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("HealthBar: Cannot update progress bar.")
        );
        return;
    }

    const float HealthPercentage = CurrentHealth / MaxHealth;

    HealthProgressBar->SetPercent(
        FMath::Clamp(HealthPercentage, 0.0f, 1.0f)
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("HealthBar: Updated to %.1f / %.1f (%.1f%%)"),
        CurrentHealth,
        MaxHealth,
        HealthPercentage * 100.0f
    );
}
