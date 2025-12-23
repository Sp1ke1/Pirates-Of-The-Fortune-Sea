#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GDS_2025/Lobby/Presets/CharacterPresetRecord.h"
#include "PlayerPresetSaveGame.generated.h"

UCLASS()
class UPlayerPresetSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Player-created presets.
	UPROPERTY(VisibleAnywhere, Category="Presets")
	TArray<FCharacterPresetRecord> UserPresets;

	// Optional: versioning for migrations later.
	UPROPERTY(VisibleAnywhere, Category="Presets")
	int32 SaveVersion = 1;
};
