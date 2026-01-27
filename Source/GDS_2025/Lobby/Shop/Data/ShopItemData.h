#pragma once

#include "CoreMinimal.h"
#include "ShopItemData.generated.h"

class UTexture2D;
class UCustomizationSlotDataAsset;

/**
 * Item available for purchase in the shop
 */
USTRUCT(BlueprintType)
struct FShopItemData
{
	GENERATED_BODY()

	// Display name of the shop item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Item")
	FText DisplayName;

	// Description text shown on detail screen
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Item")
	FText Description;

	// Preview image shown in pack list and detail screen
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Item", meta=(AllowedClasses="Texture2D"))
	TSoftObjectPtr<UTexture2D> PreviewImage;

	// Price to purchase this item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Item")
	int32 Price = 0;

	// Customization slot data asset that contains the variations available after purchase
	// The items in this slot's Items array are what the player gets when purchasing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Item")
	TSoftObjectPtr<UCustomizationSlotDataAsset> CustomizationSlotDataAsset;

	// Slot index in the CustomizationSlotDataAsset (which slot's items are available as variations)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Item", meta=(ClampMin="0"))
	int32 SlotIndex = 0;

	// Specific item indices to show from the slot (if empty, shows all items from the slot)
	// If this array is populated, only items at these indices will be displayed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Item", meta=(ClampMin="0"))
	TArray<int32> ItemIndices;
};

/**
 * Pack of items grouped by theme (e.g., "Naruto Pack")
 */
USTRUCT(BlueprintType)
struct FShopPackData
{
	GENERATED_BODY()

	// Pack name/theme (e.g., "Naruto Pack")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Pack")
	FText PackName;

	// Items in this pack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Pack")
	TArray<FShopItemData> Items;
};
