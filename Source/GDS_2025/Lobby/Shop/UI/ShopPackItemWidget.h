#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GDS_2025/Lobby/Shop/Data/ShopItemData.h"
#include "ShopPackItemWidget.generated.h"

class UShopPackItemEntryWidget;
class UTextBlock;
class UHorizontalBox;
class UPanelWidget;

/**
 * Widget for a single shop pack in the main shop list
 * Contains a pack name and horizontal list of item entries
 */
UCLASS()
class UShopPackItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Initialize this pack widget with pack data
	UFUNCTION(BlueprintCallable, Category="Shop")
	void InitializePack(const FShopPackData& InPackData, int32 InPackIndex);

	// Event dispatched when an item in this pack is clicked
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPackItemClicked, int32, PackIndex, int32, ItemIndex);

	UPROPERTY(BlueprintAssignable, Category="Shop")
	FOnPackItemClicked OnPackItemClicked;

protected:
	// Widget bindings (set in Blueprint)
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> PackNameText = nullptr;

	// Container for item entries (should be HorizontalBox or similar)
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UPanelWidget> ItemsContainer = nullptr;

	// Class of item entry widget to spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shop")
	TSubclassOf<UShopPackItemEntryWidget> ItemEntryWidgetClass = nullptr;

private:
	FShopPackData PackData;
	int32 PackIndex = INDEX_NONE;

	// Current item entry widgets (for cleanup)
	TArray<TObjectPtr<UShopPackItemEntryWidget>> ItemEntryWidgets;

	// Handler for item entry clicks (must be UFUNCTION for dynamic delegates)
	UFUNCTION()
	void OnItemEntryClicked(int32 ItemIndex);

	void ClearItems();
	void RefreshItems();
};
