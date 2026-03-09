#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomizationTabWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCustomizationTabClicked, int32, SlotIndex);

/**
 * Custom widget for a tab button in the customization edit menu
 */
UCLASS(Abstract)
class GDS_2025_API UCustomizationTabWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Initialize this tab with slot name and index
	UFUNCTION(BlueprintCallable, Category="Customization")
	void InitializeTab(const FText& InSlotName, int32 InSlotIndex);

	// Event dispatched when tab is clicked
	UPROPERTY(BlueprintAssignable, Category="Customization")
	FOnCustomizationTabClicked OnTabClicked;

	// Set selected state (this tab is currently active)
	UFUNCTION(BlueprintCallable, Category="Customization")
	void SetSelected(bool bSelected);

	// Get slot index
	UFUNCTION(BlueprintCallable, Category="Customization")
	int32 GetSlotIndex() const { return SlotIndex; }

protected:
	UPROPERTY(BlueprintReadOnly, Category="Customization")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category="Customization")
	FText SlotName;

	UPROPERTY(BlueprintReadOnly, Category="Customization")
	bool bIsSelected = false;

	// Call this from Blueprint (e.g., when the internal button is clicked)
	UFUNCTION(BlueprintCallable, Category="Customization")
	void HandleTabClicked();

	// Update visual state based on selection
	UFUNCTION(BlueprintImplementableEvent, Category="Customization")
	void BP_UpdateVisualState(bool bSelected);
	
	// Called to update visuals when initialized
	UFUNCTION(BlueprintImplementableEvent, Category="Customization")
	void BP_OnInitialized();
};
