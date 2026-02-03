#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomizationNameDialogWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;

/**
 * Dialog widget for entering preset name when saving
 */
UCLASS()
class UCustomizationNameDialogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// Initialize the dialog with default name
	UFUNCTION(BlueprintCallable, Category="Customization")
	void InitializeDialog(const FText& DefaultName = FText::GetEmpty());

	// Button handlers
	UFUNCTION(BlueprintCallable, Category="Customization")
	void OnConfirmButtonClicked();

	UFUNCTION(BlueprintCallable, Category="Customization")
	void OnCancelButtonClicked();

	// Get the entered name
	UFUNCTION(BlueprintCallable, Category="Customization")
	FText GetEnteredName() const;

	// Event dispatched when user confirms (with entered name)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNameConfirmed, FText, PresetName);

	UPROPERTY(BlueprintAssignable, Category="Customization")
	FOnNameConfirmed OnNameConfirmed;

	// Event dispatched when user cancels
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNameCancelled);

	UPROPERTY(BlueprintAssignable, Category="Customization")
	FOnNameCancelled OnNameCancelled;

protected:
	// Widget bindings (set in Blueprint)
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UEditableTextBox> NameTextBox = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> ConfirmButton = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> CancelButton = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> TitleText = nullptr;
};

