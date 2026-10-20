
#include "HealthPickupSpawnPoint.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"

AHealthPickupSpawnPoint::AHealthPickupSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(
        TEXT("SceneRoot")
    );
    RootComponent = SceneRoot;

    EditorArrow = CreateDefaultSubobject<UArrowComponent>(
        TEXT("EditorArrow")
    );
    EditorArrow->SetupAttachment(RootComponent);
    EditorArrow->SetHiddenInGame(true);

    bEnabled = true;
}
