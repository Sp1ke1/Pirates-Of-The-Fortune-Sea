#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GDS_2025/Shop/Data/ShopItemData.h"
#include "ShopPackItemEntryWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UBorder;

/**
 * Widget for a single shop item entry in a pack's horizontal list
 * Shows preview image and name, handles click to open detail view
 */
UCLASS()
class UShopPackItemEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// Initialize this entry with shop item data
	UFUNCTION(BlueprintCallable, Category="Shop")
	void InitializeItem(const FShopItemData& InItem, int32 InItemIndex);

	// Get the item data
	UFUNCTION(BlueprintCallable, Category="Shop")
	const FShopItemData& GetItem() const { return Item; }

	// Get item index
	UFUNCTION(BlueprintCallable, Category="Shop")
	int32 GetItemIndex() const { return ItemIndex; }

	// Event dispatched when item is clicked
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemClicked, int32, ItemIndex);

	UPROPERTY(BlueprintAssignable, Category="Shop")
	FOnItemClicked OnItemClicked;

protected:
	// Widget bindings (set in Blueprint)
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> ItemButton = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> ItemNameText = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UImage> ItemImage = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UBorder> ItemBorder = nullptr;

	UFUNCTION(BlueprintCallable, Category="Shop")
	void OnItemButtonClicked();

private:
	FShopItemData Item;
	int32 ItemIndex = INDEX_NONE;
};
