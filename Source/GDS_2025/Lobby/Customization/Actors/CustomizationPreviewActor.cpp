#include "GDS_2025/Lobby/Customization/Actors/CustomizationPreviewActor.h"
#include "GDS_2025/Lobby/Presets/PresetLibrarySubsystem.h"
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
}

void ACustomizationPreviewActor::ApplyPresetById(const FGuid& PresetId)
{
	if (!CharacterMesh)
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPresetLibrarySubsystem* Lib = GI->GetSubsystem<UPresetLibrarySubsystem>())
		{
			Lib->ApplyPresetToMeshById(PresetId, CharacterMesh);
		}
	}
}

