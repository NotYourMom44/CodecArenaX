#include "DefaultWeapon.h"

ADefaultWeapon::ADefaultWeapon()
{
    Damage = 3.0f;
}

void ADefaultWeapon::Fire()
{
    Super::Fire();
}