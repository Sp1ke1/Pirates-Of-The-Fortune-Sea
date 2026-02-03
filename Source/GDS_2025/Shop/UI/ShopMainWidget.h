#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopMainWidget.generated.h"

class UScrollBox;
class UPanelWidget;
class UShopPackItemWidget;
class UShopPackDataAsset;
class UShopItemDetailWidget;
class ACustomizationPreviewActor;

/**
 * Main shop widget showing list of packs
 * Each pack contains horizontal list of items
 */
UCLASS()
class UShopMainWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Initialize the widget with shop data asset and preview actor
	UFUNCTION(BlueprintCallable, Category="Shop")
	void InitializeWidget(UShopPackDataAsset* InShopDataAsset, ACustomizationPreviewActor* InPreviewActor);

	// Show this widget (used when returning from item detail)
	UFUNCTION(BlueprintCallable, Category="Shop")
	void ShowWidget();

	// Hide this widget (used when opening item detail)
	UFUNCTION(BlueprintCallable, Category="Shop")
	void HideWidget();

	// Event dispatched when an item is clicked (to open detail view)
	// Note: Detail widget is automatically created and shown in C++
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemClicked, int32, PackIndex, int32, ItemIndex);

	UPROPERTY(BlueprintAssignable, Category="Shop")
	FOnItemClicked OnItemClicked;

protected:
	// Widget bindings (set in Blueprint)
	// Container for pack widgets (should be ScrollBox or VerticalBox)
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UPanelWidget> PacksContainer = nullptr;

	// Class of pack item widget to spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shop")
	TSubclassOf<UShopPackItemWidget> PackItemWidgetClass = nullptr;

	// Class of item detail widget (used by Blueprint to open detail view)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Shop")
	TSubclassOf<UShopItemDetailWidget> ItemDetailWidgetClass = nullptr;

private:
	UPROPERTY()
	TObjectPtr<UShopPackDataAsset> ShopDataAsset = nullptr;

	UPROPERTY()
	TObjectPtr<ACustomizationPreviewActor> PreviewActor = nullptr;

	UPROPERTY()
	TObjectPtr<UShopItemDetailWidget> CurrentDetailWidget = nullptr;

	// Current pack item widgets (for cleanup)
	TArray<TObjectPtr<UShopPackItemWidget>> PackItemWidgets;

	// Handler for pack item clicks (must be UFUNCTION for dynamic delegates)
	UFUNCTION()
	void OnPackItemClicked(int32 PackIndex, int32 ItemIndex);

	// Handler for detail widget back button (must be UFUNCTION for dynamic delegates)
	UFUNCTION()
	void OnDetailWidgetBackClicked();

	void ClearPacks();
	void RefreshPacks();
	void OpenItemDetail(int32 PackIndex, int32 ItemIndex);
	void CloseItemDetail();
};
