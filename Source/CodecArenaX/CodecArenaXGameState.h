
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CodecArenaXGameState.generated.h"

UENUM(BlueprintType)
enum class ECodecArenaMatchState : uint8
{
    WaitingToStart,
    InProgress,
    MatchOver
};

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnMatchTimeUpdated,
    int32,
    RemainingSeconds
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnMatchStateChanged,
    ECodecArenaMatchState,
    NewMatchState
);

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

    UFUNCTION(BlueprintCallable, Category = "Match")
    void SetRemainingMatchTime(int32 NewRemainingSeconds);

    UFUNCTION(BlueprintPure, Category = "Match")
    int32 GetRemainingMatchTime() const;

    UFUNCTION(BlueprintCallable, Category = "Match")
    void SetMatchState(ECodecArenaMatchState NewMatchState);

    UFUNCTION(BlueprintPure, Category = "Match")
    ECodecArenaMatchState GetMatchState() const;

    UPROPERTY(BlueprintAssignable, Category = "Scoring")
    FOnScoresUpdated OnScoresUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Match")
    FOnMatchTimeUpdated OnMatchTimeUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Match")
    FOnMatchStateChanged OnMatchStateChanged;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoring", meta = (AllowPrivateAccess = "true"))
    TArray<FParticipantScore> ParticipantScores;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match", meta = (AllowPrivateAccess = "true"))
    int32 RemainingMatchTime = 300;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match", meta = (AllowPrivateAccess = "true"))
    ECodecArenaMatchState MatchState = ECodecArenaMatchState::WaitingToStart;

    int32 FindParticipantIndex(FName ParticipantId) const;
};
