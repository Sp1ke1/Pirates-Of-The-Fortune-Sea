#include "GDS_2025/Lobby/Customization/Actors/CustomizationPreviewActor.h"
#include "GDS_2025/Lobby/Presets/PresetLibrarySubsystem.h"
#include "GDS_2025/Lobby/Components/CharacterPresetApplierComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameInstance.h"

ACustomizationPreviewActor::ACustomizationPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	CharacterMesh->SetupAttachment(Root);

	// Create preset applier component (can be replaced with child class in Blueprint)
	PresetApplierComponent = CreateDefaultSubobject<UCharacterPresetApplierComponent>(TEXT("PresetApplierComponent"));
}

void ACustomizationPreviewActor::ApplyPresetById(const FGuid& PresetId)
{
	if (PresetApplierComponent)
	{
		PresetApplierComponent->ApplyPresetById(PresetId);
	}
}

