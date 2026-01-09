#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GDS_2025/Lobby/Customization/Data/CustomizationSlotData.h"
#include "CustomizationSlotDataAsset.generated.h"

/**
 * DataAsset containing all customization slots and their available items
 * Configure this in the editor to define which meshes/materials are available in each slot
 */
UCLASS(BlueprintType)
class UCustomizationSlotDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Customization")
	TArray<FCustomizationSlotData> Slots;
};

