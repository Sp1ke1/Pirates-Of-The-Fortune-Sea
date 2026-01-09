#include "GDS_2025/Lobby/Customization/UI/CustomizationMainMenuWidget.h"
#include "GDS_2025/Lobby/Customization/Actors/CustomizationPreviewActor.h"
#include "GDS_2025/Lobby/Presets/PresetLibrarySubsystem.h"
#include "GDS_2025/Lobby/Presets/CharacterPresetRecord.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"

void UCustomizationMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCustomizationMainMenuWidget::NativeDestruct()
{
	PreviewActor = nullptr;
	PresetLibrary = nullptr;
	Super::NativeDestruct();
}

void UCustomizationMainMenuWidget::InitializeWidget(ACustomizationPreviewActor* InPreviewActor)
{
	PreviewActor = InPreviewActor;

	if (UGameInstance* GI = GetGameInstance())
	{
		PresetLibrary = GI->GetSubsystem<UPresetLibrarySubsystem>();
	}
	RefreshPresetList();
	UpdateUIState();
}

void UCustomizationMainMenuWidget::OnCreateButtonClicked()
{
	OnCreateNewPreset.Broadcast();
}

void UCustomizationMainMenuWidget::OnEditButtonClicked()
{
	if (CurrentPresetId.IsValid())
	{
		OnOpenEditScreen.Broadcast(CurrentPresetId);
	}
}

void UCustomizationMainMenuWidget::OnDeleteButtonClicked()
{
	if (!PresetLibrary || !CurrentPresetId.IsValid())
	{
		return;
	}

	// Delete the preset
	if (PresetLibrary->DeleteUserPresetById(CurrentPresetId))
	{
		// Refresh list and select next/previous
		RefreshPresetList();
		
		if (UserPresetIds.Num() > 0)
		{
			// Select the first preset (or wrap around)
			SelectPresetAtIndex(0);
		}
		else
		{
			// No presets left
			CurrentPresetId = FGuid();
			CurrentPresetIndex = INDEX_NONE;
			UpdateUIState();
		}
	}
}

void UCustomizationMainMenuWidget::NavigateToPreviousPreset()
{
	if (UserPresetIds.Num() == 0)
	{
		return;
	}

	int32 NewIndex = CurrentPresetIndex - 1;
	if (NewIndex < 0)
	{
		NewIndex = UserPresetIds.Num() - 1;
	}

	SelectPresetAtIndex(NewIndex);
}

void UCustomizationMainMenuWidget::NavigateToNextPreset()
{
	if (UserPresetIds.Num() == 0)
	{
		return;
	}

	int32 NewIndex = CurrentPresetIndex + 1;
	if (NewIndex >= UserPresetIds.Num())
	{
		NewIndex = 0;
	}

	SelectPresetAtIndex(NewIndex);
}

FText UCustomizationMainMenuWidget::GetCurrentPresetName() const
{
	if (!PresetLibrary || !CurrentPresetId.IsValid())
	{
		return FText::GetEmpty();
	}

	const FCharacterPresetRecord* Record = PresetLibrary->FindPresetById(CurrentPresetId);
	if (Record)
	{
		return Record->DisplayName;
	}

	return FText::GetEmpty();
}

bool UCustomizationMainMenuWidget::HasPresets() const
{
	return UserPresetIds.Num() > 0;
}

void UCustomizationMainMenuWidget::UpdateUIState()
{
	UpdatePreview();
	UpdateButtonVisibility();

	// Update preset name text
	if (PresetNameText)
	{
		PresetNameText->SetText(GetCurrentPresetName());
	}
}

void UCustomizationMainMenuWidget::RefreshPresetList()
{
	if (!PresetLibrary)
	{
		UserPresetIds.Empty();
		return;
	}

	UserPresetIds = PresetLibrary->GetUserPresetIds();

	// If we have presets and no current selection, select first one
	if (UserPresetIds.Num() > 0)
	{
		// If current preset is still valid, keep it
		bool bCurrentStillValid = UserPresetIds.Contains(CurrentPresetId);
		if (!bCurrentStillValid || !CurrentPresetId.IsValid())
		{
			SelectPresetAtIndex(0);
		}
		else
		{
			// Find current index
			CurrentPresetIndex = UserPresetIds.IndexOfByKey(CurrentPresetId);
			if (CurrentPresetIndex == INDEX_NONE)
			{
				SelectPresetAtIndex(0);
			}
			else
			{
				UpdateUIState();
			}
		}
	}
	else
	{
		CurrentPresetId = FGuid();
		CurrentPresetIndex = INDEX_NONE;
		UpdateUIState();
	}
}

void UCustomizationMainMenuWidget::SelectPresetAtIndex(int32 Index)
{
	if (!UserPresetIds.IsValidIndex(Index))
	{
		return;
	}

	CurrentPresetIndex = Index;
	CurrentPresetId = UserPresetIds[Index];
	UpdateUIState();
}

void UCustomizationMainMenuWidget::UpdatePreview()
{
	if (PreviewActor && CurrentPresetId.IsValid())
	{
		PreviewActor->ApplyPresetById(CurrentPresetId);
	}
}

void UCustomizationMainMenuWidget::UpdateButtonVisibility()
{
	const bool bHasPresets = UserPresetIds.Num() > 0;

	// If no presets, only CREATE button is visible
	if (CreateButton)
	{
		CreateButton->SetVisibility(ESlateVisibility::Visible);
	}

	if (EditButton)
	{
		EditButton->SetVisibility(bHasPresets ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	if (DeleteButton)
	{
		DeleteButton->SetVisibility(bHasPresets ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	// Hide preset name container if no presets
	if (PresetNameContainer)
	{
		PresetNameContainer->SetVisibility(bHasPresets ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

