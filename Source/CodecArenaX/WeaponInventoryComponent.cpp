#include "WeaponInventoryComponent.h"

#include "BaseWeapon.h"
#include "SecondaryWeapon.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

UWeaponInventoryComponent::UWeaponInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

USceneComponent* UWeaponInventoryComponent::GetWeaponAttachComponent() const
{
    ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());

    if (!CharacterOwner)
    {
        return nullptr;
    }

    return CharacterOwner->GetMesh();
}

void UWeaponInventoryComponent::EquipDefaultWeapon()
{
    if (!DefaultWeapon)
    {
        if (!DefaultWeaponClass)
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
            USceneComponent* WeaponAttachComponent =
                GetWeaponAttachComponent();

            if (WeaponAttachComponent)
            {
                DefaultWeapon->AttachToComponent(
                    WeaponAttachComponent,
                    FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                    TEXT("WeaponSocket")
                );
            }
        }
    }

    if (!DefaultWeapon)
    {
        return;
    }

    if (SecondaryWeapon)
    {
        SecondaryWeapon->SetActorHiddenInGame(true);
    }

    DefaultWeapon->SetActorHiddenInGame(false);
    EquippedWeapon = DefaultWeapon;
}

void UWeaponInventoryComponent::AcquireSecondaryWeapon()
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

    if (SecondaryWeapon)
    {
        USceneComponent* WeaponAttachComponent =
            GetWeaponAttachComponent();

        if (WeaponAttachComponent)
        {
            SecondaryWeapon->AttachToComponent(
                WeaponAttachComponent,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                TEXT("WeaponSocket")
            );
        }

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

FString UWeaponInventoryComponent::GetEquippedWeaponName() const
{
    if (EquippedWeapon == SecondaryWeapon)
    {
        return TEXT("AK");
    }

    return TEXT("PISTOL");
}

FString UWeaponInventoryComponent::GetEquippedAmmoText() const
{
    if (EquippedWeapon == SecondaryWeapon)
    {
        ASecondaryWeapon* SecondaryWeaponInstance =
            Cast<ASecondaryWeapon>(SecondaryWeapon);

        if (SecondaryWeaponInstance)
        {
            return FString::Printf(
                TEXT("%d / %d"),
                SecondaryWeaponInstance->GetCurrentAmmo(),
                SecondaryWeaponInstance->GetMaxAmmo()
            );
        }
    }

    return TEXT("∞");
}

void UWeaponInventoryComponent::EquipSecondaryWeapon()
{
    if (!SecondaryWeapon)
    {
        return;
    }

    if (DefaultWeapon)
    {
        DefaultWeapon->SetActorHiddenInGame(true);
    }

    SecondaryWeapon->SetActorHiddenInGame(false);
    EquippedWeapon = SecondaryWeapon;
}