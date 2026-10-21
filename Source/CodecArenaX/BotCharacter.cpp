
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

	if (WeaponInventory)
	{
		WeaponInventory->EquipDefaultWeapon();
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
