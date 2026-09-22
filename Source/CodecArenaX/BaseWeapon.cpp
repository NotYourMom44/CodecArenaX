#include "BaseWeapon.h"

ABaseWeapon::ABaseWeapon()
{
    PrimaryActorTick.bCanEverTick = false;

    Damage = 0.0f;
}

void ABaseWeapon::Fire()
{}

float ABaseWeapon::GetDamage() const
{
    return Damage;
}