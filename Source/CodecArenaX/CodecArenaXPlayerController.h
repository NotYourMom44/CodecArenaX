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

public:
    UFUNCTION(BlueprintCallable, Category = "Pause")
    void ResumeGame();

    UFUNCTION(BlueprintCallable, Category = "Match")
    void RestartMatch();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> ShowLeaderboardAction = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> PauseAction = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> LeaderboardWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> LeaderboardWidget = nullptr;

    UPROPERTY()
    TObjectPtr<UUserWidget> PauseMenuWidget = nullptr;

    void ShowLeaderboard();
    void HideLeaderboard();
    void TogglePause();
};