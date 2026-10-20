
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HealthPickupSpawnPoint.generated.h"

class USceneComponent;
class UArrowComponent;

UCLASS(Blueprintable)
class CODECARENAX_API AHealthPickupSpawnPoint : public AActor
{
    GENERATED_BODY()

public:
    AHealthPickupSpawnPoint();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UArrowComponent> EditorArrow;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Pickup Spawn")
    bool bEnabled;
};
