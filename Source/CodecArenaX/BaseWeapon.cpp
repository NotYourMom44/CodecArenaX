#include "BaseWeapon.h"

#include "Camera/CameraComponent.h"
#include "Damageable.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "NiagaraComponent.h"

ABaseWeapon::ABaseWeapon()
{
    PrimaryActorTick.bCanEverTick = false;

    Damage = 0.0f;
    Range = 10000.0f;
}

void ABaseWeapon::Fire()
{
    APawn* WeaponOwner = Cast<APawn>(GetOwner());

    if (!WeaponOwner)
    {
        return;
    }

    UNiagaraComponent* MuzzleFlash = FindComponentByClass<UNiagaraComponent>();

    if (MuzzleFlash)
    {
        MuzzleFlash->Deactivate();
        MuzzleFlash->ResetSystem();
        MuzzleFlash->Activate(true);
    }

    UCameraComponent* Camera =
        WeaponOwner->FindComponentByClass<UCameraComponent>();

    if (!Camera)
    {
        return;
    }

    const FVector Start = Camera->GetComponentLocation();
    const FVector End =
        Start + (Camera->GetForwardVector() * Range);

    FHitResult HitResult;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(WeaponOwner);
    QueryParams.AddIgnoredActor(this);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        QueryParams
    );

    if (bHit && HitResult.GetActor())
    {
        if (IDamageable* Damageable =
            Cast<IDamageable>(HitResult.GetActor()))
        {
            Damageable->ReceiveDamage(Damage);
        }
    }

    DrawDebugLine(
        GetWorld(),
        Start,
        End,
        FColor::Red,
        false,
        1.0f,
        0,
        2.0f
    );
}

float ABaseWeapon::GetDamage() const
{
    return Damage;
}