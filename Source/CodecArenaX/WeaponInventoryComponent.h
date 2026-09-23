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

    void AcquireSecondaryWeapon(USceneComponent* AttachParent);

    UFUNCTION(BlueprintPure, Category = "Weapons")
    bool HasSecondaryWeapon() const;

    UFUNCTION(BlueprintPure, Category = "Weapons")
    ABaseWeapon* GetSecondaryWeapon() const;

    UFUNCTION(BlueprintPure, Category = "Weapons")
    ABaseWeapon* GetEquippedWeapon() const;

    UFUNCTION(BlueprintPure, Category = "Weapons")
    FString GetEquippedWeaponName() const;

    UFUNCTION(BlueprintPure, Category = "Weapons")
    FString GetEquippedAmmoText() const;

    void EquipDefaultWeapon();
    void EquipSecondaryWeapon();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Weapons")
    TSubclassOf<ABaseWeapon> DefaultWeaponClass;

    UPROPERTY(EditDefaultsOnly, Category = "Weapons")
    TSubclassOf<ABaseWeapon> SecondaryWeaponClass;

    UPROPERTY(VisibleInstanceOnly, Category = "Weapons")
    TObjectPtr<ABaseWeapon> DefaultWeapon;

    UPROPERTY(VisibleInstanceOnly, Category = "Weapons")
    TObjectPtr<ABaseWeapon> SecondaryWeapon;

    UPROPERTY(VisibleInstanceOnly, Category = "Weapons")
    TObjectPtr<ABaseWeapon> EquippedWeapon;
};