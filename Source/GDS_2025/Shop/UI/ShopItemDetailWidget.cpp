#include "GDS_2025/Shop/UI/ShopItemDetailWidget.h"
#include "GDS_2025/Customization/UI/CustomizationGridItemWidget.h"
#include "GDS_2025/Customization/Actors/CustomizationPreviewActor.h"
#include "GDS_2025/Customization/Data/CustomizationSlotDataAsset.h"
#include "Components/Button.h"
#include "Components/UniformGridPanel.h"
#include "Components/WrapBox.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "UObject/SoftObjectPath.h"

void UShopItemDetailWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UShopItemDetailWidget::OnBackButtonClicked);
	}

	if (BuyButton)
	{
		BuyButton->OnClicked.AddDynamic(this, &UShopItemDetailWidget::OnBuyButtonClicked);
	}
}

void UShopItemDetailWidget::NativeDestruct()
{
	ClearGrid();
	PreviewActor = nullptr;
	SlotDataAsset = nullptr;
	Super::NativeDestruct();
}

void UShopItemDetailWidget::InitializeWidget(const FShopItemData& InItemData, ACustomizationPreviewActor* InPreviewActor)
{
	ItemData = InItemData;
	PreviewActor = InPreviewActor;

	// Update UI text
	if (ItemNameText)
	{
		ItemNameText->SetText(ItemData.DisplayName);
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(ItemData.Description);
	}

	if (PriceText)
	{
		PriceText->SetText(FText::AsNumber(ItemData.Price));
	}

	// Load slot data asset and refresh grid
	LoadSlotDataAsset();
	RefreshGrid();

	// Initial preview update after setup
	UpdatePreview();
}

void UShopItemDetailWidget::ShowWidget()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UShopItemDetailWidget::HideWidget()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UShopItemDetailWidget::OnBackButtonClicked()
{
	OnBackClicked.Broadcast();
}

void UShopItemDetailWidget::OnBuyButtonClicked()
{
	// Call Blueprint implementable event - Blueprint can override this to handle purchase
	BP_OnBuyButtonClicked();
}

void UShopItemDetailWidget::LoadSlotDataAsset()
{
	// Load the customization slot data asset from ItemData
	if (ItemData.CustomizationSlotDataAsset.IsValid())
	{
		// Check if already loaded
		if (UCustomizationSlotDataAsset* AlreadyLoaded = ItemData.CustomizationSlotDataAsset.Get())
		{
			SlotDataAsset = AlreadyLoaded;
		}
		else
		{
			// Request async load
			FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
			const FSoftObjectPath AssetPath = ItemData.CustomizationSlotDataAsset.ToSoftObjectPath();
			if (AssetPath.IsValid())
			{
				TWeakObjectPtr<UShopItemDetailWidget> WeakWidget = this;
				Streamable.RequestAsyncLoad(AssetPath, FStreamableDelegate::CreateLambda([WeakWidget, AssetPath]() {
					if (!WeakWidget.IsValid())
					{
						return;
					}

					UObject* LoadedObject = AssetPath.ResolveObject();
					if (!LoadedObject)
					{
						return;
					}

					if (UCustomizationSlotDataAsset* LoadedAsset = Cast<UCustomizationSlotDataAsset>(LoadedObject))
					{
						if (UShopItemDetailWidget* WidgetPtr = WeakWidget.Get())
						{
							WidgetPtr->SlotDataAsset = LoadedAsset;
							WidgetPtr->RefreshGrid();
							WidgetPtr->UpdatePreview();
						}
					}
				}));
			}
		}
	}
}

void UShopItemDetailWidget::RefreshGrid()
{
	ClearGrid();

	if (!SlotDataAsset || !SlotDataAsset->Slots.IsValidIndex(ItemData.SlotIndex) || !GridItemsContainer)
	{
		return;
	}

	const FCustomizationSlotData& SlotData = SlotDataAsset->Slots[ItemData.SlotIndex];

	if (!GridItemWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShopItemDetailWidget] GridItemWidgetClass is not set."));
		return;
	}

	// Validate GridColumns
	if (GridColumns <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShopItemDetailWidget] GridColumns is %d, must be > 0. Using default value 4."), GridColumns);
		GridColumns = 4;
	}

	// Check if it's UniformGridPanel and clear it properly
	if (UUniformGridPanel* GridPanel = Cast<UUniformGridPanel>(GridItemsContainer))
	{
		// Clear all children from UniformGridPanel
		GridPanel->ClearChildren();
	}

	// Determine which items to display
	TArray<int32> ItemsToDisplay;
	GridIndexToSlotIndexMap.Empty();

	if (ItemData.ItemIndices.Num() > 0)
	{
		// Use specific indices if provided
		ItemsToDisplay = ItemData.ItemIndices;
		GridIndexToSlotIndexMap = ItemData.ItemIndices;
		UE_LOG(LogTemp, Log, TEXT("[ShopItemDetailWidget] RefreshGrid: Using specific item indices, displaying %d items"), ItemsToDisplay.Num());
	}
	else
	{
		// Use all items from slot
		for (int32 i = 0; i < SlotData.Items.Num(); ++i)
		{
			ItemsToDisplay.Add(i);
			GridIndexToSlotIndexMap.Add(i);
		}
		UE_LOG(LogTemp, Log, TEXT("[ShopItemDetailWidget] RefreshGrid: Using all items from slot, displaying %d items"), ItemsToDisplay.Num());
	}

	if (ItemsToDisplay.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShopItemDetailWidget] RefreshGrid: No items to display"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[ShopItemDetailWidget] RefreshGrid: Creating %d items with GridColumns=%d"), ItemsToDisplay.Num(), GridColumns);

	// Create grid items (variations that player will receive)
	for (int32 DisplayIndex = 0; DisplayIndex < ItemsToDisplay.Num(); ++DisplayIndex)
	{
		const int32 SlotItemIndex = ItemsToDisplay[DisplayIndex];

		// Validate slot item index
		if (!SlotData.Items.IsValidIndex(SlotItemIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("[ShopItemDetailWidget] RefreshGrid: Invalid slot item index %d (slot has %d items)"), SlotItemIndex, SlotData.Items.Num());
			continue;
		}

		UCustomizationGridItemWidget* ItemWidget = CreateWidget<UCustomizationGridItemWidget>(this, GridItemWidgetClass);
		if (!ItemWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ShopItemDetailWidget] RefreshGrid: Failed to create widget for display index %d (slot index %d)"), DisplayIndex, SlotItemIndex);
			continue;
		}

		// Initialize with the actual slot item (use slot index for the item data, but display index for widget)
		ItemWidget->InitializeItem(SlotData.Items[SlotItemIndex], SlotItemIndex);

		// Bind events (only hover/unhover, no click needed for shop preview)
		// Note: We need to pass the actual slot index, not display index
		// We'll use a lambda or store the mapping
		ItemWidget->OnItemHovered.AddDynamic(this, &UShopItemDetailWidget::OnGridItemHovered);
		ItemWidget->OnItemUnhovered.AddDynamic(this, &UShopItemDetailWidget::OnGridItemUnhovered);

		// Don't set selected state (these are just previews)

		// Add to container - check if it's UniformGridPanel
		if (UUniformGridPanel* GridPanel = Cast<UUniformGridPanel>(GridItemsContainer))
		{
			// For UniformGridPanel, use AddChildToUniformGrid with row/column
			const int32 Row = DisplayIndex / GridColumns;
			const int32 Column = DisplayIndex % GridColumns;
			UE_LOG(LogTemp, VeryVerbose, TEXT("[ShopItemDetailWidget] RefreshGrid: Adding item (display=%d, slot=%d) at Row=%d, Column=%d"), DisplayIndex, SlotItemIndex, Row, Column);
			GridPanel->AddChildToUniformGrid(ItemWidget, Column, Row);
		}
		else
		{
			// For WrapBox or other containers, use regular AddChild
			GridItemsContainer->AddChild(ItemWidget);
		}

		GridItemWidgets.Add(ItemWidget);
	}

	UE_LOG(LogTemp, Log, TEXT("[ShopItemDetailWidget] RefreshGrid: Created %d grid items"), GridItemWidgets.Num());

	UpdatePreview();
}

void UShopItemDetailWidget::UpdatePreview()
{
	if (!PreviewActor || !SlotDataAsset)
	{
		return;
	}

	// If hovering an item, show that. Otherwise show first displayed item by default.
	int32 PreviewItemIndex = INDEX_NONE;
	const int32 SlotIndex = ItemData.SlotIndex;

	if (HoveredItemIndex != INDEX_NONE)
	{
		PreviewItemIndex = HoveredItemIndex;
	}
	else
	{
		// Default to first displayed item
		if (GridIndexToSlotIndexMap.Num() > 0)
		{
			PreviewItemIndex = GridIndexToSlotIndexMap[0];
		}
		else if (ItemData.ItemIndices.Num() > 0)
		{
			PreviewItemIndex = ItemData.ItemIndices[0];
		}
		else
		{
			// Fallback to first item in slot
			PreviewItemIndex = 0;
		}
	}

	if (PreviewItemIndex != INDEX_NONE)
	{
		// Apply preview for the slot
		ApplyPreviewForSlot(SlotIndex, PreviewItemIndex);
	}
}

void UShopItemDetailWidget::OnGridItemHovered(int32 ItemIndex)
{
	if (HoveredItemIndex == ItemIndex)
	{
		return; // Already hovering this item
	}

	HoveredItemIndex = ItemIndex;

	// Update hover state for all items
	for (UCustomizationGridItemWidget* ItemWidget : GridItemWidgets)
	{
		if (ItemWidget)
		{
			ItemWidget->SetHovered(ItemWidget->GetItemIndex() == ItemIndex);
		}
	}

	// Apply preview for hovered item
	ApplyPreviewForSlot(ItemData.SlotIndex, ItemIndex);
}

void UShopItemDetailWidget::OnGridItemUnhovered(int32 ItemIndex)
{
	// Only clear hover if this was the currently hovered item
	if (HoveredItemIndex == ItemIndex)
	{
		HoveredItemIndex = INDEX_NONE;

		// Clear hover state for all items
		for (UCustomizationGridItemWidget* ItemWidget : GridItemWidgets)
		{
			if (ItemWidget)
			{
				ItemWidget->SetHovered(false);
			}
		}

		// Restore preview to default (first item)
		UpdatePreview();
	}
}

void UShopItemDetailWidget::ClearGrid()
{
	// Cancel any pending mesh load
	if (MeshLoadHandle.IsValid())
	{
		MeshLoadHandle->CancelHandle();
		MeshLoadHandle.Reset();
	}

	for (TObjectPtr<UCustomizationGridItemWidget>& ItemWidget : GridItemWidgets)
	{
		if (ItemWidget)
		{
			ItemWidget->RemoveFromParent();
		}
	}
	GridItemWidgets.Empty();
	GridIndexToSlotIndexMap.Empty();
	HoveredItemIndex = INDEX_NONE;
}

void UShopItemDetailWidget::ApplyPreviewForSlot(int32 SlotIndex, int32 ItemIndex)
{
	if (!PreviewActor)
	{
		return;
	}

	if (!SlotDataAsset)
	{
		return;
	}

	if (!SlotDataAsset->Slots.IsValidIndex(SlotIndex))
	{
		return;
	}

	const FCustomizationSlotData& SlotData = SlotDataAsset->Slots[SlotIndex];
	if (!SlotData.Items.IsValidIndex(ItemIndex))
	{
		return;
	}

	const FCustomizationSlotItem& Item = SlotData.Items[ItemIndex];

	// For main mesh slot (index 0), apply mesh (same logic as CustomizationEditWidget)
	if (SlotIndex == 0 && PreviewActor && PreviewActor->CharacterMesh)
	{
		// Cancel any pending load request
		if (MeshLoadHandle.IsValid())
		{
			MeshLoadHandle->CancelHandle();
			MeshLoadHandle.Reset();
		}

		const FSoftObjectPath MeshPath = Item.Mesh.ToSoftObjectPath();

		// Check if mesh is already loaded
		if (USkeletalMesh* AlreadyLoaded = Item.Mesh.Get())
		{
			PreviewActor->CharacterMesh->SetSkeletalMesh(AlreadyLoaded);
		}
		// Check if path is valid for async loading
		else if (MeshPath.IsValid())
		{
			// Store weak references for the callback
			TWeakObjectPtr<USkeletalMeshComponent> WeakMeshComp = PreviewActor->CharacterMesh;

			// Request async load
			FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
			MeshLoadHandle = Streamable.RequestAsyncLoad(MeshPath, FStreamableDelegate::CreateLambda([WeakMeshComp, MeshPath]() {
				if (!WeakMeshComp.IsValid())
				{
					return;
				}

				UObject* LoadedObject = MeshPath.ResolveObject();
				if (!LoadedObject)
				{
					return;
				}

				if (USkeletalMesh* LoadedMesh = Cast<USkeletalMesh>(LoadedObject))
				{
					if (USkeletalMeshComponent* MeshCompPtr = WeakMeshComp.Get())
					{
						MeshCompPtr->SetSkeletalMesh(LoadedMesh);
					}
				}
			}));
		}
	}
	// For material slot and others, can be implemented later (same as CustomizationEditWidget)
}
