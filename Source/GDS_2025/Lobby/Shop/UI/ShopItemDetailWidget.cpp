#include "GDS_2025/Lobby/Shop/UI/ShopItemDetailWidget.h"
#include "GDS_2025/Lobby/Customization/UI/CustomizationGridItemWidget.h"
#include "GDS_2025/Lobby/Customization/Actors/CustomizationPreviewActor.h"
#include "GDS_2025/Lobby/Customization/Data/CustomizationSlotDataAsset.h"
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

	// Create grid items (variations that player will receive)
	for (int32 ItemIndex = 0; ItemIndex < SlotData.Items.Num(); ++ItemIndex)
	{
		UCustomizationGridItemWidget* ItemWidget = CreateWidget<UCustomizationGridItemWidget>(this, GridItemWidgetClass);
		if (!ItemWidget)
		{
			continue;
		}

		ItemWidget->InitializeItem(SlotData.Items[ItemIndex], ItemIndex);

		// Bind events (only hover/unhover, no click needed for shop preview)
		ItemWidget->OnItemHovered.AddDynamic(this, &UShopItemDetailWidget::OnGridItemHovered);
		ItemWidget->OnItemUnhovered.AddDynamic(this, &UShopItemDetailWidget::OnGridItemUnhovered);

		// Don't set selected state (these are just previews)

		// Add to container - check if it's UniformGridPanel
		if (UUniformGridPanel* GridPanel = Cast<UUniformGridPanel>(GridItemsContainer))
		{
			// For UniformGridPanel, use AddChildToUniformGrid with row/column
			const int32 Row = ItemIndex / GridColumns;
			const int32 Column = ItemIndex % GridColumns;
			GridPanel->AddChildToUniformGrid(ItemWidget, Column, Row);
		}
		else
		{
			// For WrapBox or other containers, use regular AddChild
			GridItemsContainer->AddChild(ItemWidget);
		}

		GridItemWidgets.Add(ItemWidget);
	}

	UpdatePreview();
}

void UShopItemDetailWidget::UpdatePreview()
{
	if (!PreviewActor || !SlotDataAsset)
	{
		return;
	}

	// If hovering an item, show that. Otherwise show first item by default.
	int32 PreviewItemIndex = 0; // Default to first item
	const int32 SlotIndex = ItemData.SlotIndex;

	if (HoveredItemIndex != INDEX_NONE)
	{
		PreviewItemIndex = HoveredItemIndex;
	}

	// Apply preview for the slot
	ApplyPreviewForSlot(SlotIndex, PreviewItemIndex);
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
