
#include "HealthPickup.h"

#include "BaseCharacter.h"
#include "HealthComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

AHealthPickup::AHealthPickup()
{
    PrimaryActorTick.bCanEverTick = false;

    PickupCollision = CreateDefaultSubobject<USphereComponent>(
        TEXT("PickupCollision")
    );
    RootComponent = PickupCollision;

    PickupCollision->InitSphereRadius(100.0f);
    PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupCollision->SetCollisionObjectType(ECC_WorldDynamic);
    PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupCollision->SetCollisionResponseToChannel(
        ECC_Pawn,
        ECR_Overlap
    );
    PickupCollision->SetGenerateOverlapEvents(true);

    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("PickupMesh")
    );
    PickupMesh->SetupAttachment(RootComponent);
    PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    HealAmount = 25.0f;
    RespawnDelay = 10.0f;

    bManagerControlled = false;
    bCollected = false;
}

void AHealthPickup::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("HealthPickup: BeginPlay - %s"),
        *GetName()
    );

    PickupCollision->OnComponentBeginOverlap.AddDynamic(
        this,
        &AHealthPickup::OnPickupOverlap
    );
}

void AHealthPickup::OnPickupOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (bCollected)
    {
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("HealthPickup: Overlap event fired. Other actor: %s"),
        OtherActor ? *OtherActor->GetName() : TEXT("None")
    );

    ABaseCharacter* Character = Cast<ABaseCharacter>(OtherActor);

    if (!Character)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("HealthPickup: Overlapping actor is not an ABaseCharacter")
        );
        return;
    }

    UHealthComponent* CharacterHealth = Character->GetHealthComponent();

    if (!CharacterHealth)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("HealthPickup: Character has no HealthComponent")
        );
        return;
    }

    if (!CharacterHealth->IsAlive())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("HealthPickup: Character is not alive")
        );
        return;
    }

    if (CharacterHealth->GetCurrentHealth() >=
        CharacterHealth->GetMaxHealth())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("HealthPickup: Character already has full health")
        );
        return;
    }

    Character->Heal(HealAmount);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("HealthPickup: Healed character by %.1f"),
        HealAmount
    );

    DeactivatePickup();
    bCollected = true;

    if (bManagerControlled)
    {
        OnCollected.Broadcast(this);
    }
    else
    {
        GetWorldTimerManager().SetTimer(
            RespawnTimerHandle,
            this,
            &AHealthPickup::RespawnPickup,
            RespawnDelay,
            false
        );
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("HealthPickup: Collected. Manager controlled: %s"),
        bManagerControlled ? TEXT("true") : TEXT("false")
    );
}

void AHealthPickup::ActivateAtLocation(const FVector& NewLocation)
{
    GetWorldTimerManager().ClearTimer(RespawnTimerHandle);

    SetActorLocation(NewLocation);
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);

    PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    bCollected = false;
}

void AHealthPickup::DeactivatePickup()
{
    GetWorldTimerManager().ClearTimer(RespawnTimerHandle);

    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    PickupCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AHealthPickup::SetManagerControlled(bool bControlled)
{
    bManagerControlled = bControlled;

    if (bManagerControlled)
    {
        GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
    }
}

void AHealthPickup::RespawnPickup()
{
    if (bManagerControlled)
    {
        return;
    }

    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    bCollected = false;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("HealthPickup: Respawned - %s"),
        *GetName()
    );
}
