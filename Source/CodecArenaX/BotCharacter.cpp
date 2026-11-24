
#include "BotCharacter.h"

#include "WeaponInventoryComponent.h"
#include "BotHealthBarWidget.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Math/RotationMatrix.h"

ABotCharacter::ABotCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    WeaponInventory =
        CreateDefaultSubobject<UWeaponInventoryComponent>(
            TEXT("WeaponInventory")
        );

    HealthBarWidget =
        CreateDefaultSubobject<UWidgetComponent>(
            TEXT("HealthBarWidget")
        );

    HealthBarWidget->SetupAttachment(RootComponent);
    HealthBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));
    HealthBarWidget->SetWidgetSpace(EWidgetSpace::World);
    HealthBarWidget->SetDrawSize(FVector2D(120.0f, 12.0f));
    HealthBarWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABotCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (ParticipantId.IsNone())
    {
        ParticipantId = GetFName();

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Bot participant ID assigned: %s -> %s"),
            *GetName(),
            *ParticipantId.ToString()
        );
    }

    if (WeaponInventory)
    {
        WeaponInventory->EquipDefaultWeapon();
    }

    if (HealthBarWidget)
    {
        UBotHealthBarWidget* HealthBar =
            Cast<UBotHealthBarWidget>(
                HealthBarWidget->GetUserWidgetObject()
            );

        if (HealthBar)
        {
            HealthBar->InitializeHealthBar(GetHealthComponent());
        }
        else
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("Health bar widget is missing or has the wrong class on %s"),
                *GetName()
            );
        }
    }
}

FName ABotCharacter::GetParticipantId() const
{
    return ParticipantId;
}

void ABotCharacter::SetParticipantId(FName NewParticipantId)
{
    if (!NewParticipantId.IsNone())
    {
        ParticipantId = NewParticipantId;
    }
}

void ABotCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HealthBarWidget)
    {
        return;
    }

    APlayerCameraManager* CameraManager =
        UGameplayStatics::GetPlayerCameraManager(this, 0);

    if (!CameraManager)
    {
        return;
    }

    const FVector ToCamera =
        CameraManager->GetCameraLocation()
        - HealthBarWidget->GetComponentLocation();

    if (!ToCamera.IsNearlyZero())
    {
        const FRotator FacingRotation =
            FRotationMatrix::MakeFromX(
                ToCamera.GetSafeNormal()
            ).Rotator();

        HealthBarWidget->SetWorldRotation(FacingRotation);
    }
}

void ABotCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent
)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}
