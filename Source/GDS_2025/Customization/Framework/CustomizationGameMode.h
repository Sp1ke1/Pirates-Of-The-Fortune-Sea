#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CustomizationGameMode.generated.h"

class ACustomizationPreviewActor;
class UCustomizationMainMenuWidget;

/**
 * GameMode for customization map
 * Finds the PreviewActor in the level and provides functionality to open the main menu widget
 */
UCLASS()
class ACustomizationGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACustomizationGameMode();

	virtual void BeginPlay() override;

	// Find and cache the PreviewActor in the level
	UFUNCTION(BlueprintCallable, Category="Customization")
	void FindPreviewActor();

	// Get the PreviewActor (null if not found)
	UFUNCTION(BlueprintCallable, Category="Customization")
	ACustomizationPreviewActor* GetPreviewActor() const { return PreviewActor; }

	// Blueprint hook: open the main menu widget
	// Implement this in Blueprint to create and show the widget
	UFUNCTION(BlueprintImplementableEvent, Category="Customization")
	void BP_OpenMainMenuWidget();

protected:
	UPROPERTY()
	TObjectPtr<ACustomizationPreviewActor> PreviewActor = nullptr;
};

