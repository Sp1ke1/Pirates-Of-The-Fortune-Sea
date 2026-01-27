#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GDS_2025/Lobby/Shop/Data/ShopItemData.h"
#include "ShopItemDetailWidget.generated.h"

class UButton;
class UTextBlock;
class UUniformGridPanel;
class UWrapBox;
class UPanelWidget;
class UCustomizationGridItemWidget;
class ACustomizationPreviewActor;
class UCustomizationSlotDataAsset;
struct FStreamableHandle;

/**
 * Widget showing details of a shop item
 * Contains description, buy button, and grid of variations that player will receive
 * Reuses grid item widget and preview logic from CustomizationEditWidget
 */
UCLASS()
class UShopItemDetailWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Initialize widget with shop item data and preview actor
	UFUNCTION(BlueprintCallable, Category="Shop")
	void InitializeWidget(const FShopItemData& InItemData, ACustomizationPreviewActor* InPreviewActor);

	// Show this widget (used when opening from shop main widget)
	UFUNCTION(BlueprintCallable, Category="Shop")
	void ShowWidget();

	// Hide this widget (used when returning to shop main widget)
	UFUNCTION(BlueprintCallable, Category="Shop")
	void HideWidget();

	// Button handlers
	UFUNCTION(BlueprintCallable, Category="Shop")
	void OnBackButtonClicked();

	// Blueprint can override this to handle purchase logic
	UFUNCTION(BlueprintImplementableEvent, Category="Shop")
	void BP_OnBuyButtonClicked();
	
	// Internal function - called from button binding
	UFUNCTION(BlueprintCallable, Category="Shop")
	void OnBuyButtonClicked();

	// Event dispatched when back button is clicked
	// Note: Main widget automatically handles this in C++, but event is still available for Blueprint
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackClicked);

	UPROPERTY(BlueprintAssignable, Category="Shop")
	FOnBackClicked OnBackClicked;

protected:
	// Widget bindings (set in Blueprint)
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> BackButton = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> BuyButton = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> ItemNameText = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> DescriptionText = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> PriceText = nullptr;

	// Container for grid items (UniformGridPanel or WrapBox recommended)
	// If UniformGridPanel: will use AddChildToUniformGrid with row/column
	// If WrapBox or other: will use AddChild
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<class UPanelWidget> GridItemsContainer = nullptr;

	// Number of columns for UniformGridPanel (set in Blueprint or use default)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shop|Grid", meta=(ClampMin="1"))
	int32 GridColumns = 4;

	// Class of grid item widget to spawn (should be same as CustomizationEditWidget uses)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shop")
	TSubclassOf<UCustomizationGridItemWidget> GridItemWidgetClass = nullptr;

private:
	FShopItemData ItemData;

	UPROPERTY()
	TObjectPtr<ACustomizationPreviewActor> PreviewActor = nullptr;

	UPROPERTY()
	TObjectPtr<UCustomizationSlotDataAsset> SlotDataAsset = nullptr;

	// Currently hovered item index in grid (for preview) - this is the actual slot item index
	int32 HoveredItemIndex = INDEX_NONE;

	// Current grid item widgets (for cleanup)
	TArray<TObjectPtr<UCustomizationGridItemWidget>> GridItemWidgets;

	// Map from grid display index to actual slot item index
	// Used when ItemIndices array is specified to map displayed items to actual slot indices
	TArray<int32> GridIndexToSlotIndexMap;

	// Handle for async mesh loading (to cancel previous request if needed)
	TSharedPtr<struct FStreamableHandle> MeshLoadHandle;

	void LoadSlotDataAsset();
	void RefreshGrid();
	void UpdatePreview();

	// Handlers for grid item events (must be UFUNCTION for dynamic delegates)
	UFUNCTION()
	void OnGridItemHovered(int32 ItemIndex);

	UFUNCTION()
	void OnGridItemUnhovered(int32 ItemIndex);

	void ClearGrid();
	void ApplyPreviewForSlot(int32 SlotIndex, int32 ItemIndex);
};
