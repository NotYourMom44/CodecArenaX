
#include "HealthComponent.h"

#include "GameFramework/Controller.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    bDeathEventSent = false;
}

void UHealthComponent::TakeDamage(float DamageAmount, AController* DamageInstigator)
{
    if (DamageAmount <= 0.0f || !IsAlive())
    {
        return;
    }

    LastDamageInstigator = DamageInstigator;

    CurrentHealth = FMath::Clamp(
        CurrentHealth - DamageAmount,
        0.0f,
        MaxHealth
    );

    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("HealthComponent: %s health is now %.1f / %.1f after %.1f damage"),
        *GetNameSafe(GetOwner()),
        CurrentHealth,
        MaxHealth,
        DamageAmount
    );

    if (CurrentHealth <= 0.0f && !bDeathEventSent)
    {
        bDeathEventSent = true;
        OnHealthDepleted.Broadcast();
    }
}

void UHealthComponent::Heal(float HealAmount)
{
    if (HealAmount <= 0.0f || !IsAlive())
    {
        return;
    }

    CurrentHealth = FMath::Clamp(
        CurrentHealth + HealAmount,
        0.0f,
        MaxHealth
    );

    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::ResetHealth()
{
    CurrentHealth = MaxHealth;
    LastDamageInstigator = nullptr;
    bDeathEventSent = false;

    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

float UHealthComponent::GetCurrentHealth() const
{
    return CurrentHealth;
}

float UHealthComponent::GetMaxHealth() const
{
    return MaxHealth;
}

bool UHealthComponent::IsAlive() const
{
    return CurrentHealth > 0.0f;
}

AController* UHealthComponent::GetLastDamageInstigator() const
{
    return LastDamageInstigator.Get();
}
