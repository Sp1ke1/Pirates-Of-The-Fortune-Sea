#include "GDS_2025/Lobby/Customization/UI/CustomizationGridItemWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"

void UCustomizationGridItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemButton)
	{
		ItemButton->OnClicked.AddDynamic(this, &UCustomizationGridItemWidget::OnItemButtonClicked);
	}
}

void UCustomizationGridItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	SetHovered(true);
	OnItemHovered.Broadcast(ItemIndex);
}

void UCustomizationGridItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	
	if (bIsHovered)
	{
		SetHovered(false);
		OnItemUnhovered.Broadcast(ItemIndex);
	}
}

void UCustomizationGridItemWidget::InitializeItem(const FCustomizationSlotItem& InItem, int32 InItemIndex)
{
	Item = InItem;
	ItemIndex = InItemIndex;

	if (ItemNameText)
	{
		ItemNameText->SetText(Item.DisplayName);
	}

	UpdateVisualState();
}

void UCustomizationGridItemWidget::SetSelected(bool bSelected)
{
	if (bIsSelected == bSelected)
	{
		return;
	}

	bIsSelected = bSelected;
	UpdateVisualState();
}

void UCustomizationGridItemWidget::SetHovered(bool bHovered)
{
	if (bIsHovered == bHovered)
	{
		return;
	}

	bIsHovered = bHovered;
	UpdateVisualState();
}

void UCustomizationGridItemWidget::OnItemButtonClicked()
{
	OnItemClicked.Broadcast(ItemIndex);
}

void UCustomizationGridItemWidget::UpdateVisualState()
{
	BP_UpdateVisualState(bIsHovered, bIsSelected);
	// Note: BP_UpdateVisualState parameters are renamed to avoid conflict with class members
}

