#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GDS_2025/Lobby/Customization/Data/CustomizationSlotData.h"
#include "GDS_2025/Lobby/Presets/CharacterPresetRecord.h"
#include "CustomizationEditWidget.generated.h"

class UButton;
class UUniformGridPanel;
class UWrapBox;
class UCustomizationGridItemWidget;
class ACustomizationPreviewActor;
class UCustomizationSlotDataAsset;
class UPresetLibrarySubsystem;
struct FStreamableHandle;
class UTextBlock;

/**
 * Main editing widget for creating/editing presets
 * Contains tabs (slots) and grid of items for each slot
 */
UCLASS()
class UCustomizationEditWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Initialize widget for creating a new preset
	UFUNCTION(BlueprintCallable, Category="Customization")
	void InitializeForNewPreset(ACustomizationPreviewActor* InPreviewActor, UCustomizationSlotDataAsset* InSlotDataAsset);

	// Initialize widget for editing an existing preset
	UFUNCTION(BlueprintCallable, Category="Customization")
	void InitializeForEditPreset(ACustomizationPreviewActor* InPreviewActor, UCustomizationSlotDataAsset* InSlotDataAsset, const FGuid& PresetId);

	// Button handlers
	UFUNCTION(BlueprintCallable, Category="Customization")
	void OnCancelButtonClicked();

	// Blueprint can override this to show name dialog before saving
	UFUNCTION(BlueprintImplementableEvent, Category="Customization")
	void BP_OnSaveButtonClicked();
	
	// Internal function - called from button binding
	UFUNCTION(BlueprintCallable, Category="Customization")
	void OnSaveButtonClicked();

	// Tab (slot) selection
	UFUNCTION(BlueprintCallable, Category="Customization")
	void SelectSlot(int32 SlotIndex);

	// Navigate tabs (for RB/LB in future)
	UFUNCTION(BlueprintCallable, Category="Customization")
	void NavigateToPreviousSlot();

	UFUNCTION(BlueprintCallable, Category="Customization")
	void NavigateToNextSlot();

	// Helper function to save preset with a name (call this from name dialog)
	UFUNCTION(BlueprintCallable, Category="Customization")
	void SavePresetWithName(const FText& PresetName);

	// Get current preset name (if editing) or empty (if creating new)
	UFUNCTION(BlueprintCallable, Category="Customization")
	FText GetCurrentPresetName() const;

	// Handler for tab button clicks
	// Note: In Blueprint, bind each button directly to SelectSlot with the appropriate index
	// This function is a fallback that tries to find the clicked button
	UFUNCTION()
	void OnTabButtonClicked();

	// Event dispatched when editing is cancelled
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEditCancelled);

	UPROPERTY(BlueprintAssignable, Category="Customization")
	FOnEditCancelled OnEditCancelled;

	// Event dispatched when preset is saved (with the new/updated preset ID)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPresetSaved, FGuid, PresetId);

	UPROPERTY(BlueprintAssignable, Category="Customization")
	FOnPresetSaved OnPresetSaved;

protected:
	// Widget bindings (set in Blueprint)
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> CancelButton = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> SaveButton = nullptr;

	// Container for tab buttons (can be HorizontalBox, WrapBox, etc.)
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<class UPanelWidget> TabButtonsContainer = nullptr;

	// Container for grid items (UniformGridPanel or WrapBox recommended)
	// If UniformGridPanel: will use AddChildToUniformGrid with row/column
	// If WrapBox or other: will use AddChild
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<class UPanelWidget> GridItemsContainer = nullptr;

	// Number of columns for UniformGridPanel (set in Blueprint or use default)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Customization|Grid", meta=(ClampMin="1"))
	int32 GridColumns = 4;

	// Class of grid item widget to spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Customization")
	TSubclassOf<UCustomizationGridItemWidget> GridItemWidgetClass = nullptr;

	// Blueprint hook: create tab button widget (override in BP to customize)
	UFUNCTION(BlueprintImplementableEvent, Category="Customization")
	UButton* BP_CreateTabButton(const FText& SlotName, int32 SlotIndex);

	// Blueprint hook: setup tab button (override in BP to customize)
	UFUNCTION(BlueprintImplementableEvent, Category="Customization")
	void BP_SetupTabButton(UButton* TabButton, int32 SlotIndex, bool bIsSelected);

private:
	UPROPERTY()
	TObjectPtr<ACustomizationPreviewActor> PreviewActor = nullptr;

	UPROPERTY()
	TObjectPtr<UCustomizationSlotDataAsset> SlotDataAsset = nullptr;

	UPROPERTY()
	TObjectPtr<UPresetLibrarySubsystem> PresetLibrary = nullptr;

	// Currently selected slot index
	int32 SelectedSlotIndex = 0;

	// Currently hovered item index in current slot (for preview)
	int32 HoveredItemIndex = INDEX_NONE;

	// Map of slot index -> selected item index
	TMap<int32, int32> SelectedItemIndices;

	// Map of slot index -> tab button
	TMap<int32, TObjectPtr<UButton>> TabButtons;

	// Current grid item widgets (for cleanup)
	TArray<TObjectPtr<UCustomizationGridItemWidget>> GridItemWidgets;

	// Preset ID if editing (invalid if creating new)
	FGuid EditingPresetId;

	// Whether we're creating a new preset or editing
	bool bIsCreatingNew = true;

	// Handle for async mesh loading (to cancel previous request if needed)
	TSharedPtr<struct FStreamableHandle> MeshLoadHandle;

	void LoadSlotData();
	void RefreshGrid();
	void RefreshTabs();
	void UpdatePreview();
	
	// Handlers for grid item events (must be UFUNCTION for dynamic delegates)
	UFUNCTION()
	void OnGridItemClicked(int32 ItemIndex);
	
	UFUNCTION()
	void OnGridItemHovered(int32 ItemIndex);

	UFUNCTION()
	void OnGridItemUnhovered(int32 ItemIndex);

	// Clear hovered item (call when mouse leaves grid area)
	UFUNCTION(BlueprintCallable, Category="Customization")
	void ClearHoveredItem();
	
	void ClearGrid();
	void ApplyPreviewForSlot(int32 SlotIndex, int32 ItemIndex);
	FCharacterPresetRecord BuildPresetRecord() const;
	void LoadPresetData(const FCharacterPresetRecord& Preset);
};

