

#include "BaseCharacter.h"

ABaseCharacter::ABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(
        TEXT("HealthComponent")
    );
}

void ABaseCharacter::ReceiveDamage(float DamageAmount)
{
    if (HealthComponent)
    {
        HealthComponent->TakeDamage(DamageAmount);
    }
}

void ABaseCharacter::DebugDamage(float Amount)
{
    ReceiveDamage(Amount);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("DebugDamage: Applied %.1f damage to %s"),
        Amount,
        *GetName()
    );
}

void ABaseCharacter::Heal(float HealAmount)
{
    if (HealthComponent)
    {
        HealthComponent->Heal(HealAmount);
    }
}

UHealthComponent* ABaseCharacter::GetHealthComponent() const
{
    return HealthComponent;
}
