#include "GDS_2025/Lobby/Customization/UI/CustomizationNameDialogWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UCustomizationNameDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UCustomizationNameDialogWidget::OnConfirmButtonClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UCustomizationNameDialogWidget::OnCancelButtonClicked);
	}
}

void UCustomizationNameDialogWidget::InitializeDialog(const FText& DefaultName)
{
	if (NameTextBox)
	{
		NameTextBox->SetText(DefaultName);
		// Focus the text box (can be done in Blueprint if needed)
	}
}

void UCustomizationNameDialogWidget::OnConfirmButtonClicked()
{
	FText EnteredName = GetEnteredName();
	
	// Validate name (not empty)
	if (EnteredName.IsEmpty())
	{
		EnteredName = FText::FromString(TEXT("New Preset"));
	}

	OnNameConfirmed.Broadcast(EnteredName);
}

void UCustomizationNameDialogWidget::OnCancelButtonClicked()
{
	OnNameCancelled.Broadcast();
}

FText UCustomizationNameDialogWidget::GetEnteredName() const
{
	if (NameTextBox)
	{
		return NameTextBox->GetText();
	}

	return FText::GetEmpty();
}

