
#include "BotCharacter.h"
#include "WeaponInventoryComponent.h"

// Sets default values
ABotCharacter::ABotCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    WeaponInventory =
        CreateDefaultSubobject<UWeaponInventoryComponent>(
            TEXT("WeaponInventory")
        );
}

// Called when the game starts or when spawned
void ABotCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Assign an identity to this bot if it does not already have one.
    // Respawned bots will retain the identity of their original participant.
    if (ParticipantId.IsNone())
    {
        ParticipantId = GetFName();

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Bot participant ID assigned: %s -> %s"),
            *GetName(),
            *ParticipantId.ToString()
        );
    }

    if (WeaponInventory)
    {
        WeaponInventory->EquipDefaultWeapon();
    }
}

FName ABotCharacter::GetParticipantId() const
{
    return ParticipantId;
}

void ABotCharacter::SetParticipantId(FName NewParticipantId)
{
    if (!NewParticipantId.IsNone())
    {
        ParticipantId = NewParticipantId;
    }
}

// Called every frame
void ABotCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABotCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent
)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}
