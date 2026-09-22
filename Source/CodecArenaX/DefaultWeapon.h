#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "DefaultWeapon.generated.h"

UCLASS()
class CODECARENAX_API ADefaultWeapon : public ABaseWeapon
{
    GENERATED_BODY()

public:
    ADefaultWeapon();

    virtual void Fire() override;
};