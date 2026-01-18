#include "GDS_2025/Lobby/Shop/UI/ShopMainWidget.h"
#include "GDS_2025/Lobby/Shop/UI/ShopPackItemWidget.h"
#include "GDS_2025/Lobby/Shop/UI/ShopItemDetailWidget.h"
#include "GDS_2025/Lobby/Shop/Data/ShopPackDataAsset.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/PanelWidget.h"

void UShopMainWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UShopMainWidget::NativeDestruct()
{
	ClearPacks();
	ShopDataAsset = nullptr;
	Super::NativeDestruct();
}

void UShopMainWidget::InitializeWidget(UShopPackDataAsset* InShopDataAsset)
{
	ShopDataAsset = InShopDataAsset;
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
	OnItemClicked.Broadcast(PackIndex, ItemIndex);
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
