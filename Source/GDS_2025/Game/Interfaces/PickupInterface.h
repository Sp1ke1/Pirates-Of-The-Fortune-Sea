#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickupInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPickupInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GDS_2025_API IPickupInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * @brief Attempt to pick up the specified item.
	 * @param Item The item to pick up.
	 * @return True if successful, false otherwise.
	 */
	virtual bool AttemptPickup(AActor* Item) = 0;

	/**
	 * @brief Attempt to drop the currently held item.
	 * @return True if successful, false otherwise.
	 */
	virtual bool AttemptDrop() = 0;
};
