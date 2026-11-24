
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BotHealthBarWidget.generated.h"

class UHealthComponent;
class UProgressBar;

UCLASS()
class CODECARENAX_API UBotHealthBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeHealthBar(UHealthComponent* InHealthComponent);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> HealthProgressBar;

    UFUNCTION()
    void HandleHealthChanged(float CurrentHealth, float MaxHealth);

private:
    UPROPERTY()
    TObjectPtr<UHealthComponent> BoundHealthComponent;
};
