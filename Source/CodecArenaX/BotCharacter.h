
#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "BotCharacter.generated.h"

class UWeaponInventoryComponent;
class UWidgetComponent;

UCLASS()
class CODECARENAX_API ABotCharacter : public ABaseCharacter
{
    GENERATED_BODY()

public:
    ABotCharacter();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWeaponInventoryComponent> WeaponInventory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWidgetComponent> HealthBarWidget;

    UFUNCTION(BlueprintPure, Category = "Scoring")
    FName GetParticipantId() const;

    void SetParticipantId(FName NewParticipantId);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Scoring")
    FName ParticipantId = NAME_None;

public:
    virtual void Tick(float DeltaTime) override;

    virtual void SetupPlayerInputComponent(
        class UInputComponent* PlayerInputComponent
    ) override;
};
