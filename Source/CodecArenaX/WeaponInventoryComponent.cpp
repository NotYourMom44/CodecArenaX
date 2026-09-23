#include "WeaponInventoryComponent.h"

#include "BaseWeapon.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

UWeaponInventoryComponent::UWeaponInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponInventoryComponent::EquipDefaultWeapon(USceneComponent* AttachParent)
{
    if (!DefaultWeaponClass || DefaultWeapon)
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
        EquippedWeapon = DefaultWeapon;

        if (AttachParent)
        {
            DefaultWeapon->AttachToComponent(
                AttachParent,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale
            );
        }
    }
}

void UWeaponInventoryComponent::AcquireSecondaryWeapon(USceneComponent* AttachParent)
{
    if (!SecondaryWeaponClass || SecondaryWeapon)
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

    SecondaryWeapon = OwnerActor->GetWorld()->SpawnActor<ABaseWeapon>(
        SecondaryWeaponClass,
        FTransform::Identity,
        SpawnParameters
    );

    if (SecondaryWeapon && AttachParent)
    {
        SecondaryWeapon->AttachToComponent(
            AttachParent,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale
        );

        SecondaryWeapon->SetActorHiddenInGame(true);
    }
}

bool UWeaponInventoryComponent::HasSecondaryWeapon() const
{
    return SecondaryWeapon != nullptr;
}

ABaseWeapon* UWeaponInventoryComponent::GetSecondaryWeapon() const
{
    return SecondaryWeapon;
}

ABaseWeapon* UWeaponInventoryComponent::GetEquippedWeapon() const
{
    return EquippedWeapon;
}