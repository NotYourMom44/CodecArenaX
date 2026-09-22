#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

UCLASS()
class CODECARENAX_API ABaseWeapon : public AActor
{
    GENERATED_BODY()

public:
    ABaseWeapon();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Fire();

    UFUNCTION(BlueprintPure, Category = "Weapon")
    float GetDamage() const;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    float Damage;
};