#include "BaseCharacter.h"

ABaseCharacter::ABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ABaseCharacter::ReceiveDamage(float DamageAmount)
{
    if (HealthComponent)
    {
        HealthComponent->TakeDamage(DamageAmount);
    }
}