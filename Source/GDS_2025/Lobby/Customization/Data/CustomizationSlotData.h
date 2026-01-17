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

	// Is this item paid/premium (requires purchase)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Customization|Purchase")
	bool bIsPaid = false;

	// Price for this item (in game currency or real money, depending on your system)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Customization|Purchase")
	int32 Price = 0;
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

