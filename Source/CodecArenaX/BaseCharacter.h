#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HealthComponent.h"
#include "Damageable.h"
#include "BaseCharacter.generated.h"

UCLASS()
class CODECARENAX_API ABaseCharacter : public ACharacter, public IDamageable
{
    GENERATED_BODY()

public:
    ABaseCharacter();

    virtual void ReceiveDamage(float DamageAmount) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UHealthComponent> HealthComponent;
};