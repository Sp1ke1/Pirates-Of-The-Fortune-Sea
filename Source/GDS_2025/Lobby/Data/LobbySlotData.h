#pragma once

#include "CoreMinimal.h"
#include "LobbyTypes.h"
#include "LobbySlotData.generated.h"

USTRUCT(BlueprintType)
struct FLobbySlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	FLobbyControlAssignment Control;

	// Cosmetic / skin index for now
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	int32 SkinIndex = 0;

	// Used for focus lamp color, UI accents, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	FLinearColor PlayerColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lobby")
	bool bReady = false;
};
