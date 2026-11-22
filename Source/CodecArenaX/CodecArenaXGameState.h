
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CodecArenaXGameState.generated.h"

USTRUCT(BlueprintType)
struct FParticipantScore
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Scoring")
    FName ParticipantId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "Scoring")
    FString DisplayName;

    UPROPERTY(BlueprintReadOnly, Category = "Scoring")
    int32 Kills = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Scoring")
    bool bIsPlayer = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnScoresUpdated);

UCLASS()
class CODECARENAX_API ACodecArenaXGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Scoring")
    void RegisterParticipant(
        FName ParticipantId,
        const FString& DisplayName,
        bool bIsPlayer
    );

    UFUNCTION(BlueprintCallable, Category = "Scoring")
    void AddKill(FName ParticipantId);

    UFUNCTION(BlueprintPure, Category = "Scoring")
    TArray<FParticipantScore> GetParticipantScores() const;

    UFUNCTION(BlueprintCallable, Category = "Scoring")
    void ResetScores();

    UPROPERTY(BlueprintAssignable, Category = "Scoring")
    FOnScoresUpdated OnScoresUpdated;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoring", meta = (AllowPrivateAccess = "true"))
    TArray<FParticipantScore> ParticipantScores;

    int32 FindParticipantIndex(FName ParticipantId) const;
};
