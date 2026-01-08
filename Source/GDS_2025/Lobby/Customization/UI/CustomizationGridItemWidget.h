#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GDS_2025/Lobby/Customization/Data/CustomizationSlotData.h"
#include "CustomizationGridItemWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UBorder;

/**
 * Widget for a single item in the customization grid
 * Has two states: Hovered (showing on preview) and Selected (selected for preset)
 */
UCLASS()
class UCustomizationGridItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	// Initialize this item with slot item data
	UFUNCTION(BlueprintCallable, Category="Customization")
	void InitializeItem(const FCustomizationSlotItem& InItem, int32 InItemIndex);

	// Set selected state (this item is selected for the current preset)
	UFUNCTION(BlueprintCallable, Category="Customization")
	void SetSelected(bool bSelected);

	// Set hovered state (this item is currently being previewed on the actor)
	UFUNCTION(BlueprintCallable, Category="Customization")
	void SetHovered(bool bHovered);

	// Get the item data
	UFUNCTION(BlueprintCallable, Category="Customization")
	const FCustomizationSlotItem& GetItem() const { return Item; }

	// Get item index
	UFUNCTION(BlueprintCallable, Category="Customization")
	int32 GetItemIndex() const { return ItemIndex; }

	// Event dispatched when item is clicked
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemClicked, int32, ItemIndex);

	UPROPERTY(BlueprintAssignable, Category="Customization")
	FOnItemClicked OnItemClicked;

	// Event dispatched when item is hovered (for preview)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemHovered, int32, ItemIndex);

	UPROPERTY(BlueprintAssignable, Category="Customization")
	FOnItemHovered OnItemHovered;

	// Event dispatched when item is no longer hovered
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUnhovered, int32, ItemIndex);

	UPROPERTY(BlueprintAssignable, Category="Customization")
	FOnItemUnhovered OnItemUnhovered;

protected:
	// Widget bindings (set in Blueprint)
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> ItemButton = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> ItemNameText = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UBorder> ItemBorder = nullptr;

	// Optional: image to show mesh preview (can be set up in Blueprint)
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UImage> ItemImage = nullptr;

	UFUNCTION(BlueprintCallable, Category="Customization")
	void OnItemButtonClicked();

	// Update visual state based on hover/selected
	UFUNCTION(BlueprintImplementableEvent, Category="Customization")
	void BP_UpdateVisualState(bool bHovered, bool bSelected);

private:
	FCustomizationSlotItem Item;
	int32 ItemIndex = INDEX_NONE;
	bool bIsSelected = false;
	bool bIsHovered = false;

	void UpdateVisualState();
};

