#include "GDS_2025/Customization/Presets/CharacterPresetRecord.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/AssetManager.h"
#include "UObject/SoftObjectPath.h"


void FCharacterPresetRecord::ApplyToMeshComponent(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp)
	{
		return;
	}

	// If mesh already resident, apply immediately
	if (USkeletalMesh* Loaded = MainMesh.Get())
	{
		MeshComp->SetSkeletalMesh(Loaded);
		return;
	}

	// Otherwise request async load for this mesh and apply when ready.
	const FSoftObjectPath Path = MainMesh.ToSoftObjectPath();
	if (!Path.IsValid())
	{
		return;
	}

	TWeakObjectPtr<USkeletalMeshComponent> WeakMeshComp = MeshComp;
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(Path, FStreamableDelegate::CreateLambda([WeakMeshComp, Path]() {
		if (!WeakMeshComp.IsValid())
		{
			return;
		}

		UObject* Obj = Path.ResolveObject();
		if (!Obj)
		{
			return;
		}

		if (USkeletalMesh* LoadedMesh = Cast<USkeletalMesh>(Obj))
		{
			if (USkeletalMeshComponent* MeshCompPtr = WeakMeshComp.Get())
			{
				MeshCompPtr->SetSkeletalMesh(LoadedMesh);
			}
		}
	}));
}

