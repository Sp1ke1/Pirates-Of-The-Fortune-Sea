#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GDS_2025/Customization/Presets/CharacterPresetRecord.h"
#include "GDS_2025/Customization/Data/CustomizationSlotData.h"
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

	// Reference to the hat/helmet mesh component (optional)
	// Set via Initialize() function, not in editor
	UPROPERTY(BlueprintReadOnly, Category="Preset Applier")
	TObjectPtr<UStaticMeshComponent> HatMeshComponent = nullptr;

	// Reference to the earring static mesh component (optional)
	UPROPERTY(BlueprintReadOnly, Category="Preset Applier")
	TObjectPtr<UStaticMeshComponent> EarringMeshComponent = nullptr;

	// Reference to the pipe static mesh component (optional)
	UPROPERTY(BlueprintReadOnly, Category="Preset Applier")
	TObjectPtr<UStaticMeshComponent> PipeMeshComponent = nullptr;

	// Reference to the glasses static mesh component (optional)
	UPROPERTY(BlueprintReadOnly, Category="Preset Applier")
	TObjectPtr<UStaticMeshComponent> GlassesMeshComponent = nullptr;

	/**
	 * Initialize the component with references to mesh components from the owner actor.
	 * Call this function from the owner actor's BeginPlay to set up the component.
	 * 
	 * @param InMainMeshComponent The main character skeletal mesh component
	 * @param InHatMeshComponent The hat/helmet mesh component (optional, can be nullptr)
	 * @param InEarringMeshComponent The earring mesh component (optional, can be nullptr)
	 * @param InPipeMeshComponent The pipe mesh component (optional, can be nullptr)
	 * @param InGlassesMeshComponent The glasses mesh component (optional, can be nullptr)
	 */
	UFUNCTION(BlueprintCallable, Category="Preset Applier")
	void Initialize(USkeletalMeshComponent* InMainMeshComponent, UStaticMeshComponent* InHatMeshComponent = nullptr, UStaticMeshComponent* InEarringMeshComponent = nullptr, UStaticMeshComponent* InPipeMeshComponent = nullptr, UStaticMeshComponent* InGlassesMeshComponent = nullptr);

	/**
	 * Apply a preset by ID. This will look up the preset and call ApplyPreset.
	 */
	UFUNCTION(BlueprintCallable, Category="Preset Applier")
	bool ApplyPresetById(const FGuid& PresetId);

	/**
	 * Apply appearance from lobby slot by index.
	 * Gets the preset ID from the specified slot in ULobbyGameInstance and applies it.
	 * 
	 * @param SlotIndex Index of the slot in the lobby game instance's Slots array
	 * @return true if the preset was successfully applied, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category="Preset Applier")
	bool ApplyAppearance(int32 SlotIndex);

	/**
	 * Apply a preset record. 
	 * This is a BlueprintNativeEvent - you can override ApplyPreset in Blueprint (not _Implementation).
	 * The default C++ implementation applies the main mesh to MainMeshComponent.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Preset Applier")
	void ApplyPreset(const FCharacterPresetRecord& Preset);
	virtual void ApplyPreset_Implementation(const FCharacterPresetRecord& Preset);

	/**
	 * Apply a single customization item (for preview during editing).
	 * This is a BlueprintNativeEvent - you can override ApplyItem in Blueprint (not _Implementation).
	 * The default C++ implementation applies meshes/materials based on SlotType.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Preset Applier")
	void ApplyItem(const FCustomizationSlotItem& Item, int32 SlotIndex);
	virtual void ApplyItem_Implementation(const FCustomizationSlotItem& Item, int32 SlotIndex);

protected:
	virtual void BeginPlay() override;
};

