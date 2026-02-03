#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GDS_2025/Lobby/Presets/CharacterPresetRecord.h"
#include "CharacterPresetApplierComponent.generated.h"

class USkeletalMeshComponent;

/**
 * Base component for applying character presets to mesh components.
 * Inherit from this component in Blueprint to customize how presets are applied.
 * 
 * Usage:
 * 1. Call Initialize() from the owner actor's BeginPlay to set mesh component references
 * 2. Override ApplyPreset (not ApplyPreset_Implementation) in Blueprint to customize preset application logic
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UCharacterPresetApplierComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterPresetApplierComponent(const FObjectInitializer& ObjectInitializer);

	// Reference to the main character skeletal mesh component
	// Set via Initialize() function, not in editor
	UPROPERTY(BlueprintReadOnly, Category="Preset Applier")
	TObjectPtr<USkeletalMeshComponent> MainMeshComponent = nullptr;

	// Reference to the hat/helmet skeletal mesh component (optional)
	// Set via Initialize() function, not in editor
	UPROPERTY(BlueprintReadOnly, Category="Preset Applier")
	TObjectPtr<UStaticMeshComponent> HatMeshComponent = nullptr;

	/**
	 * Initialize the component with references to mesh components from the owner actor.
	 * Call this function from the owner actor's BeginPlay to set up the component.
	 * 
	 * @param InMainMeshComponent The main character skeletal mesh component
	 * @param InHatMeshComponent The hat/helmet mesh component (optional, can be nullptr)
	 */
	UFUNCTION(BlueprintCallable, Category="Preset Applier")
	void Initialize(USkeletalMeshComponent* InMainMeshComponent, UStaticMeshComponent* InHatMeshComponent = nullptr);

	/**
	 * Apply a preset by ID. This will look up the preset and call ApplyPreset.
	 */
	UFUNCTION(BlueprintCallable, Category="Preset Applier")
	bool ApplyPresetById(const FGuid& PresetId);

	/**
	 * Apply a preset record. 
	 * This is a BlueprintNativeEvent - you can override ApplyPreset in Blueprint (not _Implementation).
	 * The default C++ implementation applies the main mesh to MainMeshComponent.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Preset Applier")
	void ApplyPreset(const FCharacterPresetRecord& Preset);
	virtual void ApplyPreset_Implementation(const FCharacterPresetRecord& Preset);

protected:
	virtual void BeginPlay() override;
};


