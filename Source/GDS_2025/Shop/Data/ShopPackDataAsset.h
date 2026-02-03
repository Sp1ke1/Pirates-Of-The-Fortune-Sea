#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GDS_2025/Shop/Data/ShopItemData.h"
#include "ShopPackDataAsset.generated.h"

/**
 * DataAsset containing all shop packs and their items
 * Configure this in the editor to define which items are available for purchase
 */
UCLASS(BlueprintType)
class UShopPackDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	TArray<FShopPackData> Packs;
};
