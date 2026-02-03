#include "GDS_2025/Shop/UI/ShopPackItemWidget.h"
#include "GDS_2025/Shop/UI/ShopPackItemEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/PanelWidget.h"

void UShopPackItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UShopPackItemWidget::NativeDestruct()
{
	ClearItems();
	Super::NativeDestruct();
}

void UShopPackItemWidget::InitializePack(const FShopPackData& InPackData, int32 InPackIndex)
{
	PackData = InPackData;
	PackIndex = InPackIndex;

	if (PackNameText)
	{
		PackNameText->SetText(PackData.PackName);
	}

	RefreshItems();
}

void UShopPackItemWidget::RefreshItems()
{
	ClearItems();

	if (!ItemsContainer || !ItemEntryWidgetClass)
	{
		return;
	}

	// Create item entry widgets
	for (int32 ItemIndex = 0; ItemIndex < PackData.Items.Num(); ++ItemIndex)
	{
		UShopPackItemEntryWidget* ItemEntryWidget = CreateWidget<UShopPackItemEntryWidget>(this, ItemEntryWidgetClass);
		if (!ItemEntryWidget)
		{
			continue;
		}

		ItemEntryWidget->InitializeItem(PackData.Items[ItemIndex], ItemIndex);

		// Bind item clicked event
		ItemEntryWidget->OnItemClicked.AddDynamic(this, &UShopPackItemWidget::OnItemEntryClicked);

		// Add to container
		ItemsContainer->AddChild(ItemEntryWidget);

		ItemEntryWidgets.Add(ItemEntryWidget);
	}
}

void UShopPackItemWidget::OnItemEntryClicked(int32 ItemIndex)
{
	OnPackItemClicked.Broadcast(PackIndex, ItemIndex);
}

void UShopPackItemWidget::ClearItems()
{
	for (TObjectPtr<UShopPackItemEntryWidget>& ItemEntryWidget : ItemEntryWidgets)
	{
		if (ItemEntryWidget)
		{
			ItemEntryWidget->RemoveFromParent();
		}
	}
	ItemEntryWidgets.Empty();
}
