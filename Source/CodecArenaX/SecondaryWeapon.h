#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "SecondaryWeapon.generated.h"

UCLASS()
class CODECARENAX_API ASecondaryWeapon : public ABaseWeapon
{
    GENERATED_BODY()

public:
    ASecondaryWeapon();

    virtual void Fire() override;

    UFUNCTION(BlueprintPure, Category = "Weapon")
    int32 GetCurrentAmmo() const;

    UFUNCTION(BlueprintPure, Category = "Weapon")
    int32 GetMaxAmmo() const;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void AddAmmo(int32 Amount);

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
    int32 MaxAmmo;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
    int32 CurrentAmmo;
};