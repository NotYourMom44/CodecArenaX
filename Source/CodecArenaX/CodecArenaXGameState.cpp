
#include "CodecArenaXGameState.h"

int32 ACodecArenaXGameState::FindParticipantIndex(FName ParticipantId) const
{
    for (int32 Index = 0; Index < ParticipantScores.Num(); Index++)
    {
        if (ParticipantScores[Index].ParticipantId == ParticipantId)
        {
            return Index;
        }
    }

    return INDEX_NONE;
}

void ACodecArenaXGameState::RegisterParticipant(
    FName ParticipantId,
    const FString& DisplayName,
    bool bIsPlayer
)
{
    if (ParticipantId.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("Scoring: Cannot register participant with an empty ID."));
        return;
    }

    const int32 ExistingIndex = FindParticipantIndex(ParticipantId);

    if (ExistingIndex != INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Scoring: Participant %s is already registered."), *ParticipantId.ToString());
        return;
    }

    FParticipantScore NewParticipant;
    NewParticipant.ParticipantId = ParticipantId;
    NewParticipant.DisplayName = DisplayName;
    NewParticipant.Kills = 0;
    NewParticipant.bIsPlayer = bIsPlayer;

    ParticipantScores.Add(NewParticipant);

    UE_LOG(LogTemp, Warning, TEXT("Scoring: Registered participant %s (%s)."),
        *ParticipantId.ToString(), *DisplayName);

    OnScoresUpdated.Broadcast();
}

void ACodecArenaXGameState::AddKill(FName ParticipantId)
{
    const int32 Index = FindParticipantIndex(ParticipantId);

    if (Index == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("Scoring: Cannot award kill. Participant %s was not registered."),
            *ParticipantId.ToString());
        return;
    }

    ParticipantScores[Index].Kills++;

    UE_LOG(LogTemp, Warning, TEXT("Scoring: %s now has %d kills."),
        *ParticipantScores[Index].DisplayName,
        ParticipantScores[Index].Kills);

    OnScoresUpdated.Broadcast();
}

TArray<FParticipantScore> ACodecArenaXGameState::GetParticipantScores() const
{
    // Return a copy so sorting does not change the stored participant data.
    TArray<FParticipantScore> SortedScores = ParticipantScores;

    // Highest kill count first. Display name breaks ties consistently.
    SortedScores.Sort([](const FParticipantScore& A, const FParticipantScore& B)
        {
            if (A.Kills != B.Kills)
            {
                return A.Kills > B.Kills;
            }

            return A.DisplayName < B.DisplayName;
        });

    return SortedScores;
}

void ACodecArenaXGameState::ResetScores()
{
    for (FParticipantScore& Participant : ParticipantScores)
    {
        Participant.Kills = 0;
    }

    OnScoresUpdated.Broadcast();

    UE_LOG(LogTemp, Warning, TEXT("Scoring: All participant scores have been reset."));
}

void ACodecArenaXGameState::SetRemainingMatchTime(int32 NewRemainingSeconds)
{
    const int32 ClampedTime = FMath::Max(0, NewRemainingSeconds);

    if (RemainingMatchTime == ClampedTime)
    {
        return;
    }

    RemainingMatchTime = ClampedTime;
    OnMatchTimeUpdated.Broadcast(RemainingMatchTime);
}

int32 ACodecArenaXGameState::GetRemainingMatchTime() const
{
    return RemainingMatchTime;
}

void ACodecArenaXGameState::SetMatchState(ECodecArenaMatchState NewMatchState)
{
    if (MatchState == NewMatchState)
    {
        return;
    }

    MatchState = NewMatchState;
    OnMatchStateChanged.Broadcast(MatchState);
}

ECodecArenaMatchState ACodecArenaXGameState::GetMatchState() const
{
    return MatchState;
}
