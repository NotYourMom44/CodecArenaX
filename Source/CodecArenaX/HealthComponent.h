
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnHealthChanged,
    float, CurrentHealth,
    float, MaxHealth
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthDepleted);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CODECARENAX_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

    UFUNCTION(BlueprintCallable, Category = "Health")
    void TakeDamage(float DamageAmount, AController* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void ResetHealth();

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetCurrentHealth() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    AController* GetLastDamageInstigator() const;

    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnHealthDepleted OnHealthDepleted;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Health")
    float MaxHealth;

private:
    UPROPERTY(VisibleInstanceOnly, Category = "Health")
    float CurrentHealth;

    TWeakObjectPtr<AController> LastDamageInstigator;

    bool bDeathEventSent;
};
