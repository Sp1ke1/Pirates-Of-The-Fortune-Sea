#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomizationMainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class ACustomizationPreviewActor;
class UPresetLibrarySubsystem;

/**
 * First screen of customization: browse all appearances/presets
 * Shows CREATE, EDIT, DELETE buttons and allows cycling through presets with RB/LB
 */
UCLASS()
class UCustomizationMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Initialize the widget - should be called when opened
	UFUNCTION(BlueprintCallable, Category="Customization")
	void InitializeWidget(ACustomizationPreviewActor* InPreviewActor);

	// Button click handlers (will be bound in Blueprint)
	UFUNCTION(BlueprintCallable, Category="Customization")
	void OnCreateButtonClicked();

	UFUNCTION(BlueprintCallable, Category="Customization")
	void OnEditButtonClicked();

	UFUNCTION(BlueprintCallable, Category="Customization")
	void OnDeleteButtonClicked();

	// Navigation: previous/next preset (RB/LB)
	UFUNCTION(BlueprintCallable, Category="Customization")
	void NavigateToPreviousPreset();

	UFUNCTION(BlueprintCallable, Category="Customization")
	void NavigateToNextPreset();

	// Get current selected preset ID
	UFUNCTION(BlueprintCallable, Category="Customization")
	FGuid GetCurrentPresetId() const { return CurrentPresetId; }

	// Get current selected preset display name
	UFUNCTION(BlueprintCallable, Category="Customization")
	FText GetCurrentPresetName() const;

	// Update UI state (called when preset list changes or selection changes)
	UFUNCTION(BlueprintCallable, Category="Customization")
	void UpdateUIState();

	// Event dispatched when user wants to open edit screen
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOpenEditScreen, FGuid, PresetId);

	UPROPERTY(BlueprintAssignable, Category="Customization")
	FOnOpenEditScreen OnOpenEditScreen;

	// Event dispatched when user wants to create new preset
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateNewPreset);

	UPROPERTY(BlueprintAssignable, Category="Customization")
	FOnCreateNewPreset OnCreateNewPreset;

protected:
	// Widget bindings (set in Blueprint)
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> CreateButton = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> EditButton = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> DeleteButton = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> PresetNameText = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<class UWidget> PresetNameContainer = nullptr;

private:
	UPROPERTY()
	TObjectPtr<ACustomizationPreviewActor> PreviewActor = nullptr;

	UPROPERTY()
	TObjectPtr<UPresetLibrarySubsystem> PresetLibrary = nullptr;

	// Current selected preset ID (invalid if no presets)
	FGuid CurrentPresetId;

	// List of user preset IDs (for navigation)
	TArray<FGuid> UserPresetIds;

	// Current index in UserPresetIds array
	int32 CurrentPresetIndex = INDEX_NONE;

	void RefreshPresetList();
	void SelectPresetAtIndex(int32 Index);
	void UpdatePreview();
	void UpdateButtonVisibility();
};

