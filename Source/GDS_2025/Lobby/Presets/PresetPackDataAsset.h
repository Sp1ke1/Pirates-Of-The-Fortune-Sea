#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GDS_2025/Lobby/Presets/CharacterPresetRecord.h"
#include "PresetPackDataAsset.generated.h"

UCLASS(BlueprintType)
class UPresetPackDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// This is *the same format* as user presets.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Presets")
	TArray<FCharacterPresetRecord> Presets;
};
