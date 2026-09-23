#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "WeaponInventoryComponent.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class CODECARENAX_API APlayerCharacter : public ABaseCharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWeaponInventoryComponent> WeaponInventoryComponent;

    virtual void SetupPlayerInputComponent(
        class UInputComponent* PlayerInputComponent
    ) override;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartJump(const FInputActionValue& Value);
    void StopJump(const FInputActionValue& Value);
    void StartSprint(const FInputActionValue& Value);
    void StopSprint(const FInputActionValue& Value);
    void Fire(const FInputActionValue& Value);
    void Interact(const FInputActionValue& Value);
    void EquipDefaultWeapon(const FInputActionValue& Value);
    void EquipSecondaryWeapon(const FInputActionValue& Value);

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> PlayerMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> SprintAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> FireAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InteractAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> DefaultWeaponAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> SecondaryWeaponAction;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float SprintSpeed;
};