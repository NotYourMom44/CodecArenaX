
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CodecArenaXPlayerController.generated.h"

class UInputAction;
class UUserWidget;

UCLASS()
class CODECARENAX_API ACodecArenaXPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> ShowLeaderboardAction = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> LeaderboardWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> LeaderboardWidget = nullptr;

    void ShowLeaderboard();
    void HideLeaderboard();
};
