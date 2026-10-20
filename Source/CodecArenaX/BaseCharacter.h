

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

    UFUNCTION(Exec)
    void DebugDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintPure, Category = "Health")
    UHealthComponent* GetHealthComponent() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UHealthComponent> HealthComponent;
};
