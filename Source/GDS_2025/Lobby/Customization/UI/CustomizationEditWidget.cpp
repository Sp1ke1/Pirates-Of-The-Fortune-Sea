#include "GDS_2025/Lobby/Customization/UI/CustomizationEditWidget.h"
#include "GDS_2025/Lobby/Customization/UI/CustomizationGridItemWidget.h"
#include "GDS_2025/Lobby/Customization/Actors/CustomizationPreviewActor.h"
#include "GDS_2025/Lobby/Customization/Data/CustomizationSlotDataAsset.h"
#include "GDS_2025/Lobby/Presets/PresetLibrarySubsystem.h"
#include "GDS_2025/Lobby/Presets/CharacterPresetRecord.h"
#include "Components/Button.h"
#include "Components/UniformGridPanel.h"
#include "Components/WrapBox.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PanelWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "UObject/SoftObjectPath.h"

void UCustomizationEditWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		PresetLibrary = GI->GetSubsystem<UPresetLibrarySubsystem>();
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UCustomizationEditWidget::OnCancelButtonClicked);
	}

	if (SaveButton)
	{
		SaveButton->OnClicked.AddDynamic(this, &UCustomizationEditWidget::OnSaveButtonClicked);
	}
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

	SelectedSlotIndex = SlotIndex;
	HoveredItemIndex = INDEX_NONE;
	RefreshTabs();
	RefreshGrid();
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

void UCustomizationEditWidget::OnTabButtonClicked()
{
	// This is a fallback - in Blueprint, bind each button directly to SelectSlot with index
	// For C++ fallback, we can't easily determine which button was clicked from OnClicked
	// So this function is mostly a placeholder
	// In Blueprint implementation, bind: TabButton->OnClicked->SelectSlot(SlotIndex)
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

	// Clear existing buttons
	for (auto& Pair : TabButtons)
	{
		if (Pair.Value)
		{
			Pair.Value->RemoveFromParent();
		}
	}
	TabButtons.Empty();

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

		// Setup button
		BP_SetupTabButton(TabButton, SlotIndex, SelectedSlotIndex == SlotIndex);

		// Note: For proper slot identification, bind in Blueprint:
		// Each tab button should call SelectSlot with its slot index directly in BP_SetupTabButton

		// Add button to container
		if (TabButtonsContainer)
		{
			TabButtonsContainer->AddChild(TabButton);
		}
		TabButtons.Add(SlotIndex, TabButton);
	}
}

void UCustomizationEditWidget::UpdatePreview()
{
	if (!PreviewActor || !SlotDataAsset)
	{
		return;
	}

	// If hovering an item in main mesh slot (slot 0), show that. Otherwise show selected item.
	int32 PreviewItemIndex = INDEX_NONE;
	const int32 MainMeshSlotIndex = 0; // Main mesh is always slot 0
	
	// Check if we're hovering an item in the main mesh slot
	if (HoveredItemIndex != INDEX_NONE && SelectedSlotIndex == MainMeshSlotIndex)
	{
		PreviewItemIndex = HoveredItemIndex;
	}
	else
	{
		// Use selected item
		const int32* SelectedItemIndex = SelectedItemIndices.Find(MainMeshSlotIndex);
		if (SelectedItemIndex && *SelectedItemIndex != INDEX_NONE)
		{
			PreviewItemIndex = *SelectedItemIndex;
		}
	}

	if (PreviewItemIndex == INDEX_NONE || !SlotDataAsset->Slots.IsValidIndex(MainMeshSlotIndex))
	{
		return;
	}

	// Apply preview for main mesh slot
	if (PreviewItemIndex != INDEX_NONE)
	{
		UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] UpdatePreview: Applying preview for main mesh slot, item index %d"), PreviewItemIndex);
		ApplyPreviewForSlot(MainMeshSlotIndex, PreviewItemIndex);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] UpdatePreview: No preview item index available"));
	}
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

	// Apply preview for hovered item (temporarily)
	// Only apply preview for main mesh slot (slot 0), other slots can be added later
	if (SelectedSlotIndex == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] OnGridItemHovered: Applying preview for slot %d, item %d"), SelectedSlotIndex, ItemIndex);
		ApplyPreviewForSlot(SelectedSlotIndex, ItemIndex);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] OnGridItemHovered: Slot %d is not main mesh (0), not applying preview"), SelectedSlotIndex);
	}
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
}

void UCustomizationEditWidget::ApplyPreviewForSlot(int32 SlotIndex, int32 ItemIndex)
{
	if (!PreviewActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: PreviewActor is null"));
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

	// For main mesh slot (index 0), apply mesh
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
			UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: Mesh already loaded, applying immediately for slot %d, item %d"), SlotIndex, ItemIndex);
			PreviewActor->CharacterMesh->SetSkeletalMesh(AlreadyLoaded);
		}
		// Check if path is valid for async loading
		else if (MeshPath.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: Starting async load for slot %d, item %d"), SlotIndex, ItemIndex);
			
			// Store weak references for the callback
			TWeakObjectPtr<USkeletalMeshComponent> WeakMeshComp = PreviewActor->CharacterMesh;
			
			// Request async load
			FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
			MeshLoadHandle = Streamable.RequestAsyncLoad(MeshPath, FStreamableDelegate::CreateLambda([WeakMeshComp, MeshPath]() {
				if (!WeakMeshComp.IsValid())
				{
					UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: MeshComp became invalid during async load"));
					return;
				}

				UObject* LoadedObject = MeshPath.ResolveObject();
				if (!LoadedObject)
				{
					UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: Failed to resolve object after async load"));
					return;
				}

				if (USkeletalMesh* LoadedMesh = Cast<USkeletalMesh>(LoadedObject))
				{
					if (USkeletalMeshComponent* MeshCompPtr = WeakMeshComp.Get())
					{
						UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: Mesh loaded asynchronously, applying to CharacterMesh"));
						MeshCompPtr->SetSkeletalMesh(LoadedMesh);
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: Loaded object is not a USkeletalMesh"));
				}
			}));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: Item.Mesh path is not valid for slot %d, item %d"), SlotIndex, ItemIndex);
		}
	}
	else
	{
		if (SlotIndex != 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: Slot %d is not main mesh slot (0), skipping"), SlotIndex);
		}
		else if (!PreviewActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: PreviewActor is null"));
		}
		else if (!PreviewActor->CharacterMesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("[CustomizationEditWidget] ApplyPreviewForSlot: PreviewActor->CharacterMesh is null"));
		}
	}
	// For material slot and others, can be implemented later
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
		for (const auto& Pair : SelectedItemIndices)
		{
			FPresetParam Param;
			Param.Key = *FString::Printf(TEXT("Slot_%d"), Pair.Key);
			Param.Type = EPresetParamType::Int;
			Param.IntValue = Pair.Value;
			Record.Params.Add(Param);
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

