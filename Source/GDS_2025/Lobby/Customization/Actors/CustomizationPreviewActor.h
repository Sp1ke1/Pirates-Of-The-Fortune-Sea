#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomizationPreviewActor.generated.h"

class USceneComponent;
class USkeletalMeshComponent;

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

	// Apply a preset to the character mesh
	UFUNCTION(BlueprintCallable, Category="Customization")
	void ApplyPresetById(const FGuid& PresetId);

private:
	void UpdateMeshFromPreset(const FGuid& PresetId);
};

