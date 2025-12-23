#pragma once

#include "CoreMinimal.h"
#include "GDS_2025/Lobby/Data/LobbyTypes.h"
#include "LobbySlotData.generated.h"

class ULobbyCharacterPreset;

USTRUCT(BlueprintType)
struct FLobbySlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	FLobbyControlAssignment Control;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	FGuid SelectedPresetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	FLinearColor PlayerColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	bool bReady = false;
};
