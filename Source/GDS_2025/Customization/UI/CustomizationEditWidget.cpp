#include "GDS_2025/Customization/UI/CustomizationEditWidget.h"
#include "GDS_2025/Customization/UI/CustomizationGridItemWidget.h"
#include "GDS_2025/Customization/Actors/CustomizationPreviewActor.h"
#include "GDS_2025/Customization/Data/CustomizationSlotDataAsset.h"
#include "GDS_2025/Customization/Presets/PresetLibrarySubsystem.h"
#include "GDS_2025/Customization/Presets/CharacterPresetRecord.h"
#include "GDS_2025/Customization/Components/CharacterPresetApplierComponent.h"
#include "Components/Button.h"
#include "Components/UniformGridPanel.h"
#include "Components/WrapBox.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PanelWidget.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"

void UCustomizationEditWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UCustomizationEditWidget::OnCancelButtonClicked);
	}

	if (SaveButton)
	{
		SaveButton->OnClicked.AddDynamic(this, &UCustomizationEditWidget::OnSaveButtonClicked);
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		PresetLibrary = GI->GetSubsystem<UPresetLibrarySubsystem>();
	}
}

FReply UCustomizationEditWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Track mouse down to identify which button will be clicked
	// This is a workaround to identify button in OnTabButtonClickedDynamic
	// We'll check which tab button is hovered (which one is under the cursor)
	
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// Find which tab button is currently hovered
		// When mouse is clicked, the button under cursor should be in hovered state
		UButton* FoundButton = nullptr;
		for (const auto& Pair : TabButtons)
		{
			if (Pair.Value && Pair.Value->IsHovered())
			{
				FoundButton = Pair.Value;
				UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] NativeOnMouseButtonDown: Found hovered button for slot %d"), Pair.Key);
				break;
			}
		}
		
		// Store the button that will be clicked
		ClickedTabButtonForHandler = FoundButton;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UCustomizationEditWidget::NativeDestruct()
{
	ClearGrid();
	PreviewActor = nullptr;
	SlotDataAsset = nullptr;
	PresetLibrary = nullptr;
	TabButtons.Reset();
	Super::NativeDestruct();
}

void UCustomizationEditWidget::InitializeForNewPreset(ACustomizationPreviewActor* InPreviewActor, UCustomizationSlotDataAsset* InSlotDataAsset)
{
	PreviewActor = InPreviewActor;
	SlotDataAsset = InSlotDataAsset;
	bIsCreatingNew = true;
	EditingPresetId = FGuid();
	SelectedItemIndices.Empty();
	SelectedSlotIndex = 0;
	HoveredItemIndex = INDEX_NONE;

	UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] InitializeForNewPreset: PreviewActor=%p, SlotDataAsset=%p"), PreviewActor.Get(), SlotDataAsset.Get());
	if (PreviewActor)
	{
		UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] InitializeForNewPreset: CharacterMesh=%p"), PreviewActor->CharacterMesh.Get());
	}

	LoadSlotData();
	RefreshTabs();
	RefreshGrid();
	
	// Initial preview update after setup
	UpdatePreview();
}

void UCustomizationEditWidget::InitializeForEditPreset(ACustomizationPreviewActor* InPreviewActor, UCustomizationSlotDataAsset* InSlotDataAsset, const FGuid& PresetId)
{
	PreviewActor = InPreviewActor;
	SlotDataAsset = InSlotDataAsset;
	bIsCreatingNew = false;
	EditingPresetId = PresetId;
	SelectedItemIndices.Empty();
	SelectedSlotIndex = 0;
	HoveredItemIndex = INDEX_NONE;

	// Load preset data
	if (PresetLibrary)
	{
		const FCharacterPresetRecord* Preset = PresetLibrary->FindPresetById(PresetId);
		if (Preset)
		{
			LoadPresetData(*Preset);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] InitializeForEditPreset: PreviewActor=%p, SlotDataAsset=%p, PresetId=%s"), 
		PreviewActor.Get(), SlotDataAsset.Get(), *PresetId.ToString());
	if (PreviewActor)
	{
		UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] InitializeForEditPreset: CharacterMesh=%p"), PreviewActor->CharacterMesh.Get());
	}

	LoadSlotData();
	RefreshTabs();
	RefreshGrid();
	
	// Initial preview update after setup
	UpdatePreview();
}

void UCustomizationEditWidget::OnCancelButtonClicked()
{
	OnEditCancelled.Broadcast();
}

void UCustomizationEditWidget::OnSaveButtonClicked()
{
	// Call Blueprint implementable event - Blueprint can override this to show name dialog
	BP_OnSaveButtonClicked();
	
	// If Blueprint didn't implement it, use default behavior (save with current/default name)
	// This serves as fallback
	if (!bIsCreatingNew && EditingPresetId.IsValid())
	{
		// If editing, save with current name (Blueprint should override this)
		FCharacterPresetRecord PresetRecord = BuildPresetRecord();
		if (PresetLibrary)
		{
			const FCharacterPresetRecord* Existing = PresetLibrary->FindPresetById(EditingPresetId);
			if (Existing)
			{
				PresetRecord.DisplayName = Existing->DisplayName;
			}
			FGuid SavedId = PresetLibrary->AddOrUpdateUserPreset(PresetRecord);
			OnPresetSaved.Broadcast(SavedId);
		}
	}
	// If creating new and Blueprint didn't implement, do nothing (Blueprint must handle it)
}

// Helper function to be called from Blueprint after name dialog confirms
void UCustomizationEditWidget::SavePresetWithName(const FText& PresetName)
{
	if (!PresetLibrary)
	{
		return;
	}

	FCharacterPresetRecord PresetRecord = BuildPresetRecord();
	PresetRecord.DisplayName = PresetName;

	FGuid SavedId = PresetLibrary->AddOrUpdateUserPreset(PresetRecord);
	OnPresetSaved.Broadcast(SavedId);
}

FText UCustomizationEditWidget::GetCurrentPresetName() const
{
	if (bIsCreatingNew || !EditingPresetId.IsValid())
	{
		return FText::GetEmpty();
	}

	if (PresetLibrary)
	{
		const FCharacterPresetRecord* Preset = PresetLibrary->FindPresetById(EditingPresetId);
		if (Preset)
		{
			return Preset->DisplayName;
		}
	}

	return FText::GetEmpty();
}

void UCustomizationEditWidget::SelectSlot(int32 SlotIndex)
{
	if (!SlotDataAsset || !SlotDataAsset->Slots.IsValidIndex(SlotIndex))
	{
		return;
	}

	// Don't do anything if already selected this slot
	if (SelectedSlotIndex == SlotIndex)
	{
		return;
	}

	SelectedSlotIndex = SlotIndex;
	HoveredItemIndex = INDEX_NONE;
	
	// Clear hover when switching slots
	ClearHoveredItem();
	
	// Update tabs to show correct selected state
	RefreshTabs();
	
	// Refresh grid to show items for the new slot
	RefreshGrid();
	
	// Update preview with selected item from new slot
	UpdatePreview();
}

void UCustomizationEditWidget::NavigateToPreviousSlot()
{
	if (!SlotDataAsset || SlotDataAsset->Slots.Num() == 0)
	{
		return;
	}

	int32 NewIndex = SelectedSlotIndex - 1;
	if (NewIndex < 0)
	{
		NewIndex = SlotDataAsset->Slots.Num() - 1;
	}

	SelectSlot(NewIndex);
}

void UCustomizationEditWidget::NavigateToNextSlot()
{
	if (!SlotDataAsset || SlotDataAsset->Slots.Num() == 0)
	{
		return;
	}

	int32 NewIndex = SelectedSlotIndex + 1;
	if (NewIndex >= SlotDataAsset->Slots.Num())
	{
		NewIndex = 0;
	}

	SelectSlot(NewIndex);
}

void UCustomizationEditWidget::OnTabButtonClickedDynamic()
{
	// Use the button reference stored in NativeOnMouseButtonDown
	if (ClickedTabButtonForHandler)
	{
		const int32* SlotIndexPtr = TabButtonSlotIndexMap.Find(ClickedTabButtonForHandler);
		if (SlotIndexPtr)
		{
			UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] OnTabButtonClickedDynamic: Button clicked, selecting slot %d"), *SlotIndexPtr);
			SelectSlot(*SlotIndexPtr);
			ClickedTabButtonForHandler = nullptr; // Clear after use
			return;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] OnTabButtonClickedDynamic: Button found but SlotIndex not in map"));
		}
	}
	
	// Fallback: Try to find button by checking all buttons' hover state
	// This is less reliable but may work if NativeOnMouseButtonDown didn't catch it
	for (const auto& Pair : TabButtons)
	{
		if (Pair.Value && Pair.Value->IsHovered())
		{
			UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] OnTabButtonClickedDynamic: Fallback - using hovered button for slot %d"), Pair.Key);
			SelectSlot(Pair.Key);
			return;
		}
	}
	
	// Last resort: log warning
	UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] OnTabButtonClickedDynamic: Could not identify clicked button"));
}

void UCustomizationEditWidget::LoadSlotData()
{
	if (!SlotDataAsset)
	{
		return;
	}

	// Initialize default selections (first item in each slot)
	for (int32 i = 0; i < SlotDataAsset->Slots.Num(); ++i)
	{
		if (!SelectedItemIndices.Contains(i))
		{
			// Default to first item (index 0) if slot has items
			const FCustomizationSlotData& SlotData = SlotDataAsset->Slots[i];
			if (SlotData.Items.Num() > 0)
			{
				SelectedItemIndices.Add(i, 0);
			}
			else
			{
				SelectedItemIndices.Add(i, INDEX_NONE);
			}
		}
	}
}

void UCustomizationEditWidget::RefreshGrid()
{
	ClearGrid();

	if (!SlotDataAsset || !SlotDataAsset->Slots.IsValidIndex(SelectedSlotIndex) || !GridItemsContainer)
	{
		return;
	}

	const FCustomizationSlotData& SlotData = SlotDataAsset->Slots[SelectedSlotIndex];

	if (!GridItemWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] GridItemWidgetClass is not set."));
		return;
	}

	// Create grid items
	for (int32 ItemIndex = 0; ItemIndex < SlotData.Items.Num(); ++ItemIndex)
	{
		UCustomizationGridItemWidget* ItemWidget = CreateWidget<UCustomizationGridItemWidget>(this, GridItemWidgetClass);
		if (!ItemWidget)
		{
			continue;
		}

		ItemWidget->InitializeItem(SlotData.Items[ItemIndex], ItemIndex);
		
		// Bind events (dynamic delegates use AddDynamic)
		ItemWidget->OnItemClicked.AddDynamic(this, &UCustomizationEditWidget::OnGridItemClicked);
		ItemWidget->OnItemHovered.AddDynamic(this, &UCustomizationEditWidget::OnGridItemHovered);
		ItemWidget->OnItemUnhovered.AddDynamic(this, &UCustomizationEditWidget::OnGridItemUnhovered);

		// Set selected state
		int32* SelectedIndex = SelectedItemIndices.Find(SelectedSlotIndex);
		ItemWidget->SetSelected(SelectedIndex && *SelectedIndex == ItemIndex);

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

void UCustomizationEditWidget::RefreshTabs()
{
	if (!SlotDataAsset || !TabButtonsContainer)
	{
		return;
	}

	// If buttons already exist, just update their visual state
	if (TabButtons.Num() > 0 && TabButtons.Num() == SlotDataAsset->Slots.Num())
	{
		// Update existing buttons' selected state
		for (const auto& Pair : TabButtons)
		{
			if (Pair.Value)
			{
				BP_SetupTabButton(Pair.Value, Pair.Key, SelectedSlotIndex == Pair.Key);
			}
		}
		return;
	}

	// Clear existing buttons (if any)
	for (auto& Pair : TabButtons)
	{
		if (Pair.Value)
		{
			Pair.Value->RemoveFromParent();
		}
	}
		TabButtons.Empty();

	// Clear button to slot index map
	TabButtonSlotIndexMap.Empty();

	// Create tab buttons
	for (int32 SlotIndex = 0; SlotIndex < SlotDataAsset->Slots.Num(); ++SlotIndex)
	{
		const FCustomizationSlotData& SlotData = SlotDataAsset->Slots[SlotIndex];
		
		UButton* TabButton = BP_CreateTabButton(SlotData.SlotName, SlotIndex);
		if (!TabButton)
		{
			// Fallback: create a simple button if BP didn't create one
			TabButton = NewObject<UButton>(this);
		}

		// Store the SlotIndex in a map for the handler to lookup
		const int32 CapturedSlotIndex = SlotIndex;
		TabButtonSlotIndexMap.Add(TabButton, CapturedSlotIndex);
		
		// Bind OnClicked - FOnButtonClickedEvent is a dynamic delegate, so we must use AddDynamic
		// Since AddDynamic doesn't support lambdas, we need a workaround to pass SlotIndex
		// Solution: Override NativeOnMouseButtonDown in a custom button widget to store button reference
		// Or use a member variable to track the last clicked button
		// For now, we'll bind and handle in OnTabButtonClickedDynamic
		
		// We'll store button reference using NativeOnMouseButtonDown override
		// But since we're using standard UButton, we'll use a workaround:
		// Create a wrapper that captures the button before binding
		TabButton->OnClicked.AddDynamic(this, &UCustomizationEditWidget::OnTabButtonClickedDynamic);
		
		// Store button reference for handler lookup - we'll set it via NativeOnMouseButtonDown
		// But since we can't override UButton, we'll use a different approach
		// For now, we'll try to identify button in handler using other means

		// Setup button (for visual state updates only)
		BP_SetupTabButton(TabButton, SlotIndex, SelectedSlotIndex == SlotIndex);

		// Add button to container
		if (TabButtonsContainer)
		{
			TabButtonsContainer->AddChild(TabButton);
		}
			TabButtons.Add(SlotIndex, TabButton);
	}
}

int32 UCustomizationEditWidget::GetSlotIndexForTabButton(UButton* TabButton) const
{
	if (!TabButton)
	{
		return INDEX_NONE;
	}

	// Search for the button in our map
	const int32* SlotIndexPtr = TabButtonSlotIndexMap.Find(TabButton);
	if (SlotIndexPtr)
	{
		return *SlotIndexPtr;
	}

	// Fallback: Search in TabButtons map
	for (const auto& Pair : TabButtons)
	{
		if (Pair.Value == TabButton)
		{
			return Pair.Key;
		}
	}

	return INDEX_NONE;
}

void UCustomizationEditWidget::SelectSlotByTabButton(UButton* TabButton)
{
	if (!TabButton)
	{
		return;
	}

	const int32 SlotIndex = GetSlotIndexForTabButton(TabButton);
	if (SlotIndex != INDEX_NONE)
	{
		SelectSlot(SlotIndex);
	}
}

void UCustomizationEditWidget::UpdatePreview()
{
	if (!PreviewActor || !SlotDataAsset)
	{
		return;
	}

	int32 PreviewItemIndex = INDEX_NONE;
	if (HoveredItemIndex != INDEX_NONE)
	{
		PreviewItemIndex = HoveredItemIndex;
	}
	else
	{
		const int32* SelectedItemIndex = SelectedItemIndices.Find(SelectedSlotIndex);
		if (SelectedItemIndex && *SelectedItemIndex != INDEX_NONE)
		{
			PreviewItemIndex = *SelectedItemIndex;
		}
	}

	if (PreviewItemIndex == INDEX_NONE || !SlotDataAsset->Slots.IsValidIndex(SelectedSlotIndex))
	{
		return;
	}

	ApplyPreviewForSlot(SelectedSlotIndex, PreviewItemIndex);
}

void UCustomizationEditWidget::OnGridItemClicked(int32 ItemIndex)
{
	SelectedItemIndices.Add(SelectedSlotIndex, ItemIndex);

	// Update grid to reflect selection
	for (UCustomizationGridItemWidget* ItemWidget : GridItemWidgets)
	{
		if (ItemWidget)
		{
			ItemWidget->SetSelected(ItemWidget->GetItemIndex() == ItemIndex);
		}
	}

	// Clear hover and show selected item
	HoveredItemIndex = INDEX_NONE;
	for (UCustomizationGridItemWidget* ItemWidget : GridItemWidgets)
	{
		if (ItemWidget)
		{
			ItemWidget->SetHovered(false);
		}
	}

	UpdatePreview();
}

void UCustomizationEditWidget::OnGridItemHovered(int32 ItemIndex)
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

	ApplyPreviewForSlot(SelectedSlotIndex, ItemIndex);
}

void UCustomizationEditWidget::OnGridItemUnhovered(int32 ItemIndex)
{
	// Only clear hover if this was the currently hovered item
	if (HoveredItemIndex == ItemIndex)
	{
		// Check if mouse moved to another item - if not, clear hover after a small delay
		// For now, immediately clear and restore to selected
		ClearHoveredItem();
	}
}

void UCustomizationEditWidget::ClearHoveredItem()
{
	if (HoveredItemIndex == INDEX_NONE)
	{
		return;
	}

	HoveredItemIndex = INDEX_NONE;

	// Clear hover state for all items
	for (UCustomizationGridItemWidget* ItemWidget : GridItemWidgets)
	{
		if (ItemWidget)
		{
			ItemWidget->SetHovered(false);
		}
	}

	// Restore preview to selected item
	UpdatePreview();
}

void UCustomizationEditWidget::ClearGrid()
{
	for (TObjectPtr<UCustomizationGridItemWidget>& ItemWidget : GridItemWidgets)
	{
		if (ItemWidget)
		{
			ItemWidget->RemoveFromParent();
		}
	}
	GridItemWidgets.Empty();
}

void UCustomizationEditWidget::ApplyPreviewForSlot(int32 SlotIndex, int32 ItemIndex)
{
	if (!PreviewActor || !PreviewActor->PresetApplierComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: PreviewActor or PresetApplierComponent is null"));
		return;
	}

	if (!SlotDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: SlotDataAsset is null"));
		return;
	}

	if (!SlotDataAsset->Slots.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: Invalid slot index %d"), SlotIndex);
		return;
	}

	const FCustomizationSlotData& SlotData = SlotDataAsset->Slots[SlotIndex];
	if (!SlotData.Items.IsValidIndex(ItemIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: Invalid item index %d for slot %d"), ItemIndex, SlotIndex);
		return;
	}

	const FCustomizationSlotItem& Item = SlotData.Items[ItemIndex];

	// Delegate to the component - all logic is centralized there and can be customized in Blueprint
	UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: Applying item for slot %d, item %d via PresetApplierComponent"), SlotIndex, ItemIndex);
	PreviewActor->PresetApplierComponent->ApplyItem(Item, SlotIndex);
}

FCharacterPresetRecord UCustomizationEditWidget::BuildPresetRecord() const
{
	FCharacterPresetRecord Record;

	if (!bIsCreatingNew && EditingPresetId.IsValid())
	{
		Record.Id = EditingPresetId;
	}
	else
	{
		Record.Id = FGuid::NewGuid();
	}

	// Set display name (default, can be overridden by name dialog)
	Record.DisplayName = FText::FromString(TEXT("New Preset"));

	// Store selected items in Params
	// Format: Key = "Slot_<SlotIndex>", Value = ItemIndex (as IntValue)
	if (SlotDataAsset)
	{
		// Clear slots array before filling
		Record.Slots.Empty();

		for (const auto& Pair : SelectedItemIndices)
		{
			FPresetParam Param;
			Param.Key = *FString::Printf(TEXT("Slot_%d"), Pair.Key);
			Param.Type = EPresetParamType::Int;
			Param.IntValue = Pair.Value;
			Record.Params.Add(Param);

			// Also store full per-slot data (snapshot of FCustomizationSlotItem)
			const int32 SlotIndex = Pair.Key;
			const int32 ItemIndex = Pair.Value;

			if (SlotDataAsset->Slots.IsValidIndex(SlotIndex))
			{
				const FCustomizationSlotData& SlotData = SlotDataAsset->Slots[SlotIndex];
				if (SlotData.Items.IsValidIndex(ItemIndex))
				{
					const FCustomizationSlotItem& Item = SlotData.Items[ItemIndex];

					FCharacterPresetSlot SlotRecord;
					SlotRecord.SlotIndex = SlotIndex;
					SlotRecord.Item = Item;

					Record.Slots.Add(SlotRecord);
				}
			}
		}

		// Set MainMesh from first slot (main mesh slot)
		if (SlotDataAsset->Slots.Num() > 0)
		{
			const int32* MainMeshItemIndex = SelectedItemIndices.Find(0);
			if (MainMeshItemIndex && *MainMeshItemIndex != INDEX_NONE)
			{
				const FCustomizationSlotData& MainMeshSlot = SlotDataAsset->Slots[0];
				if (MainMeshSlot.Items.IsValidIndex(*MainMeshItemIndex))
				{
					Record.MainMesh = MainMeshSlot.Items[*MainMeshItemIndex].Mesh;
				}
			}
		}
	}

	return Record;
}

void UCustomizationEditWidget::LoadPresetData(const FCharacterPresetRecord& Preset)
{
	if (!SlotDataAsset)
	{
		return;
	}

	SelectedItemIndices.Empty();

	// Load selected items from Params
	for (const FPresetParam& Param : Preset.Params)
	{
		if (Param.Type == EPresetParamType::Int && Param.Key.ToString().StartsWith(TEXT("Slot_")))
		{
			FString SlotIndexStr = Param.Key.ToString().RightChop(5); // Remove "Slot_"
			int32 SlotIndex = FCString::Atoi(*SlotIndexStr);
			if (SlotIndex >= 0 && SlotIndex < SlotDataAsset->Slots.Num())
			{
				SelectedItemIndices.Add(SlotIndex, Param.IntValue);
			}
		}
	}

	// If no params found, try to match MainMesh with slot items
	if (SelectedItemIndices.Num() == 0 && Preset.MainMesh.IsValid())
	{
		if (SlotDataAsset->Slots.Num() > 0)
		{
			const FCustomizationSlotData& MainMeshSlot = SlotDataAsset->Slots[0];
			for (int32 ItemIndex = 0; ItemIndex < MainMeshSlot.Items.Num(); ++ItemIndex)
			{
				if (MainMeshSlot.Items[ItemIndex].Mesh == Preset.MainMesh)
				{
					SelectedItemIndices.Add(0, ItemIndex);
					break;
				}
			}
		}
	}
}

