#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomizationPreviewActor.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class UCharacterPresetApplierComponent;

/**
 * Actor used on customization map to display the current selected preset/appearance
 */
UCLASS()
class ACustomizationPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ACustomizationPreviewActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Customization|Components")
	TObjectPtr<USceneComponent> Root = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Customization|Components")
	TObjectPtr<USkeletalMeshComponent> CharacterMesh = nullptr;

	// Component for applying character presets
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Customization|Components")
	TObjectPtr<UCharacterPresetApplierComponent> PresetApplierComponent = nullptr;

	// Apply a preset to the character mesh
	UFUNCTION(BlueprintCallable, Category="Customization")
	void ApplyPresetById(const FGuid& PresetId);
};

