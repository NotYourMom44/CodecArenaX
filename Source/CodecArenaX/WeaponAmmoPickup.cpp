#include "WeaponAmmoPickup.h"

#include "Components/StaticMeshComponent.h"
#include "PlayerCharacter.h"
#include "WeaponInventoryComponent.h"
#include "SecondaryWeapon.h"

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

void AWeaponAmmoPickup::Interact(APlayerCharacter* PlayerCharacter)
{
    if (!PlayerCharacter)
    {
        return;
    }

    UWeaponInventoryComponent* Inventory =
        PlayerCharacter->FindComponentByClass<UWeaponInventoryComponent>();

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

    Destroy();
}