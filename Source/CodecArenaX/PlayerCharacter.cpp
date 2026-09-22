#include "PlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"

APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    WalkSpeed = 600.0f;
    SprintSpeed = 1000.0f;

    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    WeaponInventoryComponent =
        CreateDefaultSubobject<UWeaponInventoryComponent>(
            TEXT("WeaponInventoryComponent")
        );
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (WeaponInventoryComponent)
    {
        UCameraComponent* CameraComponent =
            FindComponentByClass<UCameraComponent>();

        WeaponInventoryComponent->EquipDefaultWeapon(CameraComponent);
    }
}

void APlayerCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent
)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
                LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                if (PlayerMappingContext)
                {
                    InputSubsystem->AddMappingContext(PlayerMappingContext, 0);
                }
            }
        }
    }

    if (UEnhancedInputComponent* EnhancedInputComponent =
        Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(
                MoveAction,
                ETriggerEvent::Triggered,
                this,
                &APlayerCharacter::Move
            );
        }

        if (LookAction)
        {
            EnhancedInputComponent->BindAction(
                LookAction,
                ETriggerEvent::Triggered,
                this,
                &APlayerCharacter::Look
            );
        }

        if (JumpAction)
        {
            EnhancedInputComponent->BindAction(
                JumpAction,
                ETriggerEvent::Started,
                this,
                &APlayerCharacter::StartJump
            );

            EnhancedInputComponent->BindAction(
                JumpAction,
                ETriggerEvent::Completed,
                this,
                &APlayerCharacter::StopJump
            );
        }

        if (SprintAction)
        {
            EnhancedInputComponent->BindAction(
                SprintAction,
                ETriggerEvent::Started,
                this,
                &APlayerCharacter::StartSprint
            );

            EnhancedInputComponent->BindAction(
                SprintAction,
                ETriggerEvent::Completed,
                this,
                &APlayerCharacter::StopSprint
            );
        }
    }
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller)
    {
        const FRotator ControlRotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

        const FVector ForwardDirection =
            FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        const FVector RightDirection =
            FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void APlayerCharacter::StartJump(const FInputActionValue& Value)
{
    Jump();
}

void APlayerCharacter::StopJump(const FInputActionValue& Value)
{
    StopJumping();
}

void APlayerCharacter::StartSprint(const FInputActionValue& Value)
{
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void APlayerCharacter::StopSprint(const FInputActionValue& Value)
{
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
    const FVector2D LookAxis = Value.Get<FVector2D>();

    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(-LookAxis.Y);
}