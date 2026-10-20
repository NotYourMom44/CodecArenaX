
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HealthPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ABaseCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnHealthPickupCollected,
    AHealthPickup*,
    CollectedPickup
);

UCLASS()
class CODECARENAX_API AHealthPickup : public AActor
{
    GENERATED_BODY()

public:
    AHealthPickup();

    UPROPERTY(BlueprintAssignable, Category = "Pickup")
    FOnHealthPickupCollected OnCollected;

    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void ActivateAtLocation(const FVector& NewLocation);

    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void DeactivatePickup();

    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void SetManagerControlled(bool bControlled);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> PickupCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> PickupMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
    float HealAmount;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
    float RespawnDelay;

private:
    FTimerHandle RespawnTimerHandle;

    bool bManagerControlled;
    bool bCollected;

    UFUNCTION()
    void OnPickupOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComponent,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    void RespawnPickup();
};
