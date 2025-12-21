#include "GDS_2025/Lobby/Presets/CharacterPresetRecord.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"

void FCharacterPresetRecord::ApplyToMeshComponent(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp)
	{
		return;
	}

	if (!MainMesh.IsNull())
	{
		USkeletalMesh* LoadedMesh = MainMesh.LoadSynchronous();
		if (LoadedMesh)
		{
			MeshComp->SetSkeletalMesh(LoadedMesh);
		}
	}
}
