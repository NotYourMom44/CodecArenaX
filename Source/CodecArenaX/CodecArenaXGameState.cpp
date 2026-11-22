
#include "CodecArenaXGameState.h"

int32 ACodecArenaXGameState::FindParticipantIndex(
    FName ParticipantId
) const
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
        return;
    }

    const int32 ExistingIndex = FindParticipantIndex(ParticipantId);

    if (ExistingIndex != INDEX_NONE)
    {
        return;
    }

    FParticipantScore NewParticipant;
    NewParticipant.ParticipantId = ParticipantId;
    NewParticipant.DisplayName = DisplayName;
    NewParticipant.Kills = 0;
    NewParticipant.bIsPlayer = bIsPlayer;

    ParticipantScores.Add(NewParticipant);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Scoring: Registered participant %s (%s)."),
        *DisplayName,
        *ParticipantId.ToString()
    );

    OnScoresUpdated.Broadcast();
}

void ACodecArenaXGameState::AddKill(FName ParticipantId)
{
    const int32 ParticipantIndex = FindParticipantIndex(ParticipantId);

    if (ParticipantIndex == INDEX_NONE)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Scoring: Cannot award kill. Participant %s is not registered."),
            *ParticipantId.ToString()
        );

        return;
    }

    ParticipantScores[ParticipantIndex].Kills++;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Scoring: %s now has %d kills."),
        *ParticipantScores[ParticipantIndex].DisplayName,
        ParticipantScores[ParticipantIndex].Kills
    );

    OnScoresUpdated.Broadcast();
}

TArray<FParticipantScore>
ACodecArenaXGameState::GetParticipantScores() const
{
    return ParticipantScores;
}

void ACodecArenaXGameState::ResetScores()
{
    for (FParticipantScore& Participant : ParticipantScores)
    {
        Participant.Kills = 0;
    }

    UE_LOG(LogTemp, Warning, TEXT("Scoring: All participant scores reset."));

    OnScoresUpdated.Broadcast();
}
