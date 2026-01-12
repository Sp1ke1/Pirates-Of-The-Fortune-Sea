#include "GDS_2025/Lobby/Components/CharacterPresetApplierComponent.h"
#include "GDS_2025/Lobby/Presets/PresetLibrarySubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameInstance.h"

UCharacterPresetApplierComponent::UCharacterPresetApplierComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterPresetApplierComponent::BeginPlay()
{
	Super::BeginPlay();

	// References to mesh components should be set via Initialize() function
	// This is just a validation point
}

void UCharacterPresetApplierComponent::Initialize(USkeletalMeshComponent* InMainMeshComponent, USkeletalMeshComponent* InHatMeshComponent)
{
	MainMeshComponent = InMainMeshComponent;
	HatMeshComponent = InHatMeshComponent;
}

bool UCharacterPresetApplierComponent::ApplyPresetById(const FGuid& PresetId)
{
	if (!PresetId.IsValid())
	{
		return false;
	}

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI)
	{
		return false;
	}

	UPresetLibrarySubsystem* Lib = GI->GetSubsystem<UPresetLibrarySubsystem>();
	if (!Lib)
	{
		return false;
	}

	const FCharacterPresetRecord* Preset = Lib->FindPresetById(PresetId);
	if (!Preset)
	{
		return false;
	}

	ApplyPreset(*Preset);
	return true;
}

void UCharacterPresetApplierComponent::ApplyPreset_Implementation(const FCharacterPresetRecord& Preset)
{
	// Default implementation: apply main mesh to MainMeshComponent
	if (MainMeshComponent)
	{
		Preset.ApplyToMeshComponent(MainMeshComponent);
	}

	// Hat mesh and other customization can be handled in Blueprint override
}


