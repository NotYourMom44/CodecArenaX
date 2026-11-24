
#include "WeaponAmmoPickup.h"

#include "Components/StaticMeshComponent.h"
#include "BaseCharacter.h"
#include "WeaponInventoryComponent.h"
#include "SecondaryWeapon.h"
#include "TimerManager.h"

AWeaponAmmoPickup::AWeaponAmmoPickup()
{
    PrimaryActorTick.bCanEverTick = false;

    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("PickupMesh")
    );

    RootComponent = PickupMesh;

    PickupMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupMesh->SetCollisionResponseToChannel(
        ECC_Visibility,
        ECR_Block
    );

    AmmoAmount = 12;
}

void AWeaponAmmoPickup::Interact(ABaseCharacter* Character)
{
    if (!Character || !bIsAvailable)
    {
        return;
    }

    UWeaponInventoryComponent* Inventory =
        Character->FindComponentByClass<UWeaponInventoryComponent>();

    if (!Inventory)
    {
        return;
    }

    const bool bAlreadyHadSecondary =
        Inventory->HasSecondaryWeapon();

    if (!bAlreadyHadSecondary)
    {
        Inventory->AcquireSecondaryWeapon();
    }

    ABaseWeapon* SecondaryWeaponBase =
        Inventory->GetSecondaryWeapon();

    ASecondaryWeapon* SecondaryWeapon =
        Cast<ASecondaryWeapon>(SecondaryWeaponBase);

    if (bAlreadyHadSecondary && SecondaryWeapon)
    {
        SecondaryWeapon->AddAmmo(AmmoAmount);
    }

    // Temporarily disable the pickup.
    bIsAvailable = false;

    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    // Respawn after the configured delay.
    GetWorldTimerManager().SetTimer(
        RespawnTimerHandle,
        this,
        &AWeaponAmmoPickup::RespawnPickup,
        RespawnDelay,
        false
    );
}

void AWeaponAmmoPickup::RespawnPickup()
{
    bIsAvailable = true;

    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
}
