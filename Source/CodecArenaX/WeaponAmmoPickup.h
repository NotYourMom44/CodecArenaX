
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponAmmoPickup.generated.h"

class UStaticMeshComponent;
class ABaseCharacter;

UCLASS()
class CODECARENAX_API AWeaponAmmoPickup : public AActor
{
    GENERATED_BODY()

public:
    AWeaponAmmoPickup();

    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void Interact(ABaseCharacter* Character);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> PickupMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
    int32 AmmoAmount;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
    float RespawnDelay = 15.0f;

    UFUNCTION()
    void RespawnPickup();

private:
    FTimerHandle RespawnTimerHandle;

    bool bIsAvailable = true;
};
