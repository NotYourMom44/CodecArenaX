#include "SecondaryWeapon.h"

ASecondaryWeapon::ASecondaryWeapon()
{
    Damage = 12.0f;

    MaxAmmo = 24;
    CurrentAmmo = 12;
}

void ASecondaryWeapon::Fire()
{
    if (CurrentAmmo <= 0)
    {
        return;
    }

    Super::Fire();

    CurrentAmmo--;
}

int32 ASecondaryWeapon::GetCurrentAmmo() const
{
    return CurrentAmmo;
}

int32 ASecondaryWeapon::GetMaxAmmo() const
{
    return MaxAmmo;
}

void ASecondaryWeapon::AddAmmo(int32 Amount)
{
    if (Amount <= 0)
    {
        return;
    }

    CurrentAmmo = FMath::Clamp(
        CurrentAmmo + Amount,
        0,
        MaxAmmo
    );
}