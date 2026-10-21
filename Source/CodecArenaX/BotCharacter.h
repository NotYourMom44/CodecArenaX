
#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "BotCharacter.generated.h"

class UWeaponInventoryComponent;

UCLASS()
class CODECARENAX_API ABotCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ABotCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWeaponInventoryComponent> WeaponInventory;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(
		class UInputComponent* PlayerInputComponent
	) override;
};
