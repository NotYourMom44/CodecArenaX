
#include "WeaponProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Damageable.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AWeaponProjectile::AWeaponProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(
        TEXT("CollisionComponent")
    );

    SetRootComponent(CollisionComponent);

    CollisionComponent->InitSphereRadius(8.0f);
    CollisionComponent->SetCollisionProfileName(
        TEXT("BlockAllDynamic")
    );
    CollisionComponent->SetCollisionEnabled(
        ECollisionEnabled::QueryOnly
    );
    CollisionComponent->SetNotifyRigidBodyCollision(true);

    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("ProjectileMesh")
    );

    ProjectileMesh->SetupAttachment(CollisionComponent);
    ProjectileMesh->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );
    ProjectileMesh->SetRelativeScale3D(
        FVector(0.12f, 0.12f, 0.12f)
    );

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere")
    );

    if (SphereMesh.Succeeded())
    {
        ProjectileMesh->SetStaticMesh(SphereMesh.Object);
    }

    ProjectileMovement =
        CreateDefaultSubobject<UProjectileMovementComponent>(
            TEXT("ProjectileMovement")
        );

    ProjectileMovement->SetUpdatedComponent(CollisionComponent);
    ProjectileMovement->InitialSpeed = 3000.0f;
    ProjectileMovement->MaxSpeed = 3000.0f;
    ProjectileMovement->ProjectileGravityScale = 0.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    Damage = 0.0f;
}

void AWeaponProjectile::BeginPlay()
{
    Super::BeginPlay();

    CollisionComponent->OnComponentHit.AddDynamic(
        this,
        &AWeaponProjectile::OnProjectileHit
    );

    if (GetOwner())
    {
        CollisionComponent->IgnoreActorWhenMoving(
            GetOwner(),
            true
        );
    }

    SetLifeSpan(5.0f);
}

void AWeaponProjectile::InitializeProjectile(
    const FVector& Direction,
    float InDamage
)
{
    Damage = InDamage;

    const FVector NormalizedDirection = Direction.GetSafeNormal();

    if (!NormalizedDirection.IsNearlyZero())
    {
        SetActorRotation(NormalizedDirection.Rotation());

        ProjectileMovement->Velocity =
            NormalizedDirection * ProjectileMovement->InitialSpeed;
    }
}

void AWeaponProjectile::OnProjectileHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    FVector NormalImpulse,
    const FHitResult& Hit
)
{
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
    {
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Projectile hit actor: %s"),
        *GetNameSafe(OtherActor)
    );

    if (IDamageable* Damageable = Cast<IDamageable>(OtherActor))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Projectile applying %.1f damage to %s"),
            Damage,
            *GetNameSafe(OtherActor)
        );

        Damageable->ReceiveDamage(Damage);
    }
    else
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Hit actor does not implement IDamageable: %s"),
            *GetNameSafe(OtherActor)
        );
    }

    Destroy();
}
