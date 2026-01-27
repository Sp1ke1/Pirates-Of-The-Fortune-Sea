#include "GDS_2025/Lobby/Shop/UI/ShopMainWidget.h"
#include "GDS_2025/Lobby/Shop/UI/ShopPackItemWidget.h"
#include "GDS_2025/Lobby/Shop/UI/ShopItemDetailWidget.h"
#include "GDS_2025/Lobby/Shop/Data/ShopPackDataAsset.h"
#include "GDS_2025/Lobby/Customization/Actors/CustomizationPreviewActor.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/PanelWidget.h"

void UShopMainWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UShopMainWidget::NativeDestruct()
{
	CloseItemDetail();
	ClearPacks();
	ShopDataAsset = nullptr;
	PreviewActor = nullptr;
	Super::NativeDestruct();
}

void UShopMainWidget::InitializeWidget(UShopPackDataAsset* InShopDataAsset, ACustomizationPreviewActor* InPreviewActor)
{
	ShopDataAsset = InShopDataAsset;
	PreviewActor = InPreviewActor;
	RefreshPacks();
}

void UShopMainWidget::ShowWidget()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UShopMainWidget::HideWidget()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UShopMainWidget::RefreshPacks()
{
	ClearPacks();

	if (!ShopDataAsset || !PacksContainer || !PackItemWidgetClass)
	{
		return;
	}

	// Create pack item widgets
	for (int32 PackIndex = 0; PackIndex < ShopDataAsset->Packs.Num(); ++PackIndex)
	{
		const FShopPackData& PackData = ShopDataAsset->Packs[PackIndex];
		
		UShopPackItemWidget* PackItemWidget = CreateWidget<UShopPackItemWidget>(this, PackItemWidgetClass);
		if (!PackItemWidget)
		{
			continue;
		}

		PackItemWidget->InitializePack(PackData, PackIndex);

		// Bind pack item clicked event
		PackItemWidget->OnPackItemClicked.AddDynamic(this, &UShopMainWidget::OnPackItemClicked);

		// Add to container
		PacksContainer->AddChild(PackItemWidget);

		PackItemWidgets.Add(PackItemWidget);
	}
}

void UShopMainWidget::OnPackItemClicked(int32 PackIndex, int32 ItemIndex)
{
	// Broadcast event for Blueprint (if needed)
	OnItemClicked.Broadcast(PackIndex, ItemIndex);

	// Automatically open detail widget
	OpenItemDetail(PackIndex, ItemIndex);
}

void UShopMainWidget::OpenItemDetail(int32 PackIndex, int32 ItemIndex)
{
	if (!ShopDataAsset || !ShopDataAsset->Packs.IsValidIndex(PackIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShopMainWidget] OpenItemDetail: Invalid pack index %d"), PackIndex);
		return;
	}

	const FShopPackData& PackData = ShopDataAsset->Packs[PackIndex];
	if (!PackData.Items.IsValidIndex(ItemIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShopMainWidget] OpenItemDetail: Invalid item index %d for pack %d"), ItemIndex, PackIndex);
		return;
	}

	if (!ItemDetailWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShopMainWidget] OpenItemDetail: ItemDetailWidgetClass is not set."));
		return;
	}

	// Close existing detail widget if any
	CloseItemDetail();

	// Create new detail widget
	CurrentDetailWidget = CreateWidget<UShopItemDetailWidget>(this, ItemDetailWidgetClass);
	if (!CurrentDetailWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShopMainWidget] OpenItemDetail: Failed to create detail widget."));
		return;
	}

	// Get item data
	const FShopItemData& ItemData = PackData.Items[ItemIndex];

	// Initialize detail widget
	CurrentDetailWidget->InitializeWidget(ItemData, PreviewActor);

	// Bind back button event
	CurrentDetailWidget->OnBackClicked.AddDynamic(this, &UShopMainWidget::OnDetailWidgetBackClicked);

	// Add to viewport and show
	CurrentDetailWidget->AddToViewport();
	CurrentDetailWidget->ShowWidget();

	// Hide main widget
	HideWidget();
}

void UShopMainWidget::CloseItemDetail()
{
	if (CurrentDetailWidget)
	{
		CurrentDetailWidget->RemoveFromParent();
		CurrentDetailWidget = nullptr;
	}

	// Show main widget
	ShowWidget();
}

void UShopMainWidget::OnDetailWidgetBackClicked()
{
	CloseItemDetail();
}

void UShopMainWidget::ClearPacks()
{
	for (TObjectPtr<UShopPackItemWidget>& PackItemWidget : PackItemWidgets)
	{
		if (PackItemWidget)
		{
			PackItemWidget->RemoveFromParent();
		}
	}
	PackItemWidgets.Empty();
}
