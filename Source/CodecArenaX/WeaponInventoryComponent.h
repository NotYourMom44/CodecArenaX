#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponInventoryComponent.generated.h"

class ABaseWeapon;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CODECARENAX_API UWeaponInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWeaponInventoryComponent();

    void EquipDefaultWeapon(USceneComponent* AttachParent);

    UFUNCTION(BlueprintPure, Category = "Weapons")
    ABaseWeapon* GetEquippedWeapon() const;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Weapons")
    TSubclassOf<ABaseWeapon> DefaultWeaponClass;

    UPROPERTY(VisibleInstanceOnly, Category = "Weapons")
    TObjectPtr<ABaseWeapon> DefaultWeapon;

    UPROPERTY(VisibleInstanceOnly, Category = "Weapons")
    TObjectPtr<ABaseWeapon> EquippedWeapon;
};