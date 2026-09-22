#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

UINTERFACE(BlueprintType)
class CODECARENAX_API UDamageable : public UInterface
{
    GENERATED_BODY()
};

class CODECARENAX_API IDamageable
{
    GENERATED_BODY()

public:
    virtual void ReceiveDamage(float DamageAmount) = 0;
};