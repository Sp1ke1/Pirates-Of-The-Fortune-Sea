#pragma once

#include "CoreMinimal.h"
#include "CustomizationSlotData.generated.h"

class USkeletalMesh;
class UMaterialInterface;

/**
 * Item in a customization slot (e.g., a mesh or material)
 */
USTRUCT(BlueprintType)
struct FCustomizationSlotItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Customization")
	FText DisplayName;

	// Mesh for this item (used for "Main Mesh" slot)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Customization", meta=(AllowedClasses="SkeletalMesh"))
	TSoftObjectPtr<USkeletalMesh> Mesh;

	// Material for this item (used for "Material" slot)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Customization", meta=(AllowedClasses="Material,MaterialInstance"))
	TSoftObjectPtr<UMaterialInterface> Material;
};

/**
 * Data for a customization slot (e.g., "Main Mesh", "Material", "Hat", "Boots")
 */
USTRUCT(BlueprintType)
struct FCustomizationSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Customization")
	FText SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Customization")
	TArray<FCustomizationSlotItem> Items;
};

