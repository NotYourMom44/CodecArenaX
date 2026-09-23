#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponAmmoPickup.generated.h"

class UStaticMeshComponent;
class APlayerCharacter;

UCLASS()
class CODECARENAX_API AWeaponAmmoPickup : public AActor
{
    GENERATED_BODY()

public:
    AWeaponAmmoPickup();

    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void Interact(APlayerCharacter* PlayerCharacter);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> PickupMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
    int32 AmmoAmount;
};