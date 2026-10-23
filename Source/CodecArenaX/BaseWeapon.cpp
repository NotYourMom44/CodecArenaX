
#include "BaseWeapon.h"

#include "WeaponProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

ABaseWeapon::ABaseWeapon()
{
    PrimaryActorTick.bCanEverTick = false;

    Damage = 0.0f;
    Range = 10000.0f;
}

void ABaseWeapon::Fire()
{
    APawn* WeaponOwner = Cast<APawn>(GetOwner());

    if (!WeaponOwner || !ProjectileClass)
    {
        return;
    }

    USkeletalMeshComponent* WeaponMesh =
        FindComponentByClass<USkeletalMeshComponent>();

    if (!WeaponMesh)
    {
        return;
    }

    const FName MuzzleSocketName(TEXT("MuzzleSocket"));

    if (!WeaponMesh->DoesSocketExist(MuzzleSocketName))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("%s: WeaponMesh has no MuzzleSocket."),
            *GetName()
        );

        return;
    }

    const FTransform MuzzleTransform =
        WeaponMesh->GetSocketTransform(
            MuzzleSocketName,
            RTS_World
        );

    const FVector ProjectileDirection =
        MuzzleTransform.GetUnitAxis(EAxis::X);

    if (ProjectileDirection.IsNearlyZero())
    {
        return;
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = WeaponOwner;
    SpawnParameters.Instigator = WeaponOwner;
    SpawnParameters.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWeaponProjectile* Projectile =
        GetWorld()->SpawnActor<AWeaponProjectile>(
            ProjectileClass,
            MuzzleTransform.GetLocation(),
            ProjectileDirection.Rotation(),
            SpawnParameters
        );

    if (Projectile)
    {
        Projectile->InitializeProjectile(
            ProjectileDirection,
            Damage
        );
    }
}

bool ABaseWeapon::FireAtTarget(AActor* Target)
{
    APawn* WeaponOwner = Cast<APawn>(GetOwner());

    if (!WeaponOwner || !IsValid(Target) || !ProjectileClass)
    {
        return false;
    }

    USkeletalMeshComponent* WeaponMesh =
        FindComponentByClass<USkeletalMeshComponent>();

    if (!WeaponMesh)
    {
        return false;
    }

    const FName MuzzleSocketName(TEXT("MuzzleSocket"));

    if (!WeaponMesh->DoesSocketExist(MuzzleSocketName))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("%s: WeaponMesh has no MuzzleSocket."),
            *GetName()
        );

        return false;
    }

    const FTransform MuzzleTransform =
        WeaponMesh->GetSocketTransform(
            MuzzleSocketName,
            RTS_World
        );

    const FVector Direction =
        (Target->GetActorLocation() - MuzzleTransform.GetLocation())
        .GetSafeNormal();

    if (Direction.IsNearlyZero())
    {
        return false;
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = WeaponOwner;
    SpawnParameters.Instigator = WeaponOwner;
    SpawnParameters.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWeaponProjectile* Projectile =
        GetWorld()->SpawnActor<AWeaponProjectile>(
            ProjectileClass,
            MuzzleTransform.GetLocation(),
            Direction.Rotation(),
            SpawnParameters
        );

    if (Projectile)
    {
        Projectile->InitializeProjectile(Direction, Damage);
        return true;
    }

    return false;
}

float ABaseWeapon::GetDamage() const
{
    return Damage;
}
