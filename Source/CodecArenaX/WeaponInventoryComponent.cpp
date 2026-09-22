#include "WeaponInventoryComponent.h"

#include "BaseWeapon.h"
#include "GameFramework/Actor.h"

UWeaponInventoryComponent::UWeaponInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponInventoryComponent::EquipDefaultWeapon(USceneComponent* AttachParent)
{
    if (!DefaultWeaponClass || DefaultWeapon || !AttachParent)
    {
        return;
    }

    AActor* OwnerActor = GetOwner();

    if (!OwnerActor)
    {
        return;
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = OwnerActor;
    SpawnParameters.Instigator = Cast<APawn>(OwnerActor);

    DefaultWeapon = OwnerActor->GetWorld()->SpawnActor<ABaseWeapon>(
        DefaultWeaponClass,
        FTransform::Identity,
        SpawnParameters
    );

    if (DefaultWeapon)
    {
        DefaultWeapon->AttachToComponent(
            AttachParent,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale
        );

        EquippedWeapon = DefaultWeapon;
    }
}

ABaseWeapon* UWeaponInventoryComponent::GetEquippedWeapon() const
{
    return EquippedWeapon;
}