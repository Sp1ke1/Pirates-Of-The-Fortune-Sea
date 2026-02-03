#pragma once

#include "CoreMinimal.h"
#include "GDS_2025/Customization/Data/CustomizationSlotData.h"
#include "CharacterPresetRecord.generated.h"

class USkeletalMesh;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EPresetParamType : uint8
{
	Float,
	Int,
	Bool,
	Color,
	Name
};

USTRUCT(BlueprintType)
struct FPresetParam
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	FName Key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	EPresetParamType Type = EPresetParamType::Float;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	float FloatValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	bool BoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	FLinearColor ColorValue = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	FName NameValue;
};

/**
 * Saved data for a single customization slot inside a preset.
 * Содержит индекс слота и снапшот исходного FCustomizationSlotItem.
 */
USTRUCT(BlueprintType)
struct FCharacterPresetSlot
{
	GENERATED_BODY()

	// Index of customization slot in data asset
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	int32 SlotIndex = INDEX_NONE;

	// Full item data (type + meshes/materials + цена и т.п.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	FCustomizationSlotItem Item;
};

USTRUCT(BlueprintType)
struct FCharacterPresetRecord
{
	GENERATED_BODY()

	// Stable unique ID. For dev presets: set once and don't change.
	// For user presets: generate at creation time.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	FGuid Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	FText DisplayName;

	// Main mesh for now. Soft ref so it's SaveGame-friendly and loadable.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	TSoftObjectPtr<USkeletalMesh> MainMesh;

	// Detailed per-slot data (type + meshes/materials)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	TArray<FCharacterPresetSlot> Slots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preset")
	TArray<FPresetParam> Params;

	// Helper: apply to mesh (sync load is OK for lobby; can be upgraded to async later)
	void ApplyToMeshComponent(USkeletalMeshComponent* MeshComp) const;
};

