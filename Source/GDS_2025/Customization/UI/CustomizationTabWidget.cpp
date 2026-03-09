#include "GDS_2025/Customization/UI/CustomizationTabWidget.h"

void UCustomizationTabWidget::InitializeTab(const FText& InSlotName, int32 InSlotIndex)
{
	SlotName = InSlotName;
	SlotIndex = InSlotIndex;
	
	BP_OnInitialized();
}

void UCustomizationTabWidget::SetSelected(bool bSelected)
{
	if (bIsSelected != bSelected)
	{
		bIsSelected = bSelected;
		BP_UpdateVisualState(bIsSelected);
	}
}

void UCustomizationTabWidget::HandleTabClicked()
{
	OnTabClicked.Broadcast(SlotIndex);
}
