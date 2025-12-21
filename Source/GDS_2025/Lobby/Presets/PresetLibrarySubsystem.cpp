#include "GDS_2025/Lobby/Presets/PresetLibrarySubsystem.h"

#include "GDS_2025/Lobby/Presets/PresetPackDataAsset.h"
#include "GDS_2025/Lobby/Presets/PlayerPresetSaveGame.h"

#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"

void UPresetLibrarySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Load (or create) user presets on startup
	LoadUserPresets();
}

void UPresetLibrarySubsystem::SetDevPresetPacks(const TArray<UPresetPackDataAsset*>& InPacks)
{
	DevPacks = InPacks;
}

void UPresetLibrarySubsystem::EnsureSaveObject()
{
	if (SaveObject)
	{
		return;
	}

	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex);
		SaveObject = Cast<UPlayerPresetSaveGame>(Loaded);
	}

	if (!SaveObject)
	{
		SaveObject = Cast<UPlayerPresetSaveGame>(
			UGameplayStatics::CreateSaveGameObject(UPlayerPresetSaveGame::StaticClass())
		);
	}
}

bool UPresetLibrarySubsystem::LoadUserPresets()
{
	EnsureSaveObject();
	return SaveObject != nullptr;
}

bool UPresetLibrarySubsystem::SaveUserPresets()
{
	EnsureSaveObject();
	if (!SaveObject)
	{
		return false;
	}

	return UGameplayStatics::SaveGameToSlot(SaveObject, SaveSlotName, SaveUserIndex);
}

void UPresetLibrarySubsystem::CollectDevPresets(TArray<FCharacterPresetRecord>& Out) const
{
	for (const UPresetPackDataAsset* Pack : DevPacks)
	{
		if (!Pack)
		{
			continue;
		}

		for (const FCharacterPresetRecord& R : Pack->Presets)
		{
			Out.Add(R);
		}
	}
}

TArray<FCharacterPresetRecord> UPresetLibrarySubsystem::GetAllPresets() const
{
	TArray<FCharacterPresetRecord> Result;
	CollectDevPresets(Result);

	if (SaveObject)
	{
		Result.Append(SaveObject->UserPresets);
	}

	return Result;
}

TArray<FGuid> UPresetLibrarySubsystem::GetAllPresetIds() const
{
	const TArray<FCharacterPresetRecord> All = GetAllPresets();

	TArray<FGuid> Ids;
	Ids.Reserve(All.Num());

	for (const FCharacterPresetRecord& R : All)
	{
		Ids.Add(R.Id);
	}

	return Ids;
}

const FCharacterPresetRecord* UPresetLibrarySubsystem::FindPresetById(const FGuid& Id) const
{
	// Dev first
	for (const UPresetPackDataAsset* Pack : DevPacks)
	{
		if (!Pack)
		{
			continue;
		}

		for (const FCharacterPresetRecord& R : Pack->Presets)
		{
			if (R.Id == Id)
			{
				return &R;
			}
		}
	}

	// User
	if (SaveObject)
	{
		for (const FCharacterPresetRecord& R : SaveObject->UserPresets)
		{
			if (R.Id == Id)
			{
				return &R;
			}
		}
	}

	return nullptr;
}

FGuid UPresetLibrarySubsystem::AddOrUpdateUserPreset(FCharacterPresetRecord Record)
{
	EnsureSaveObject();
	if (!SaveObject)
	{
		return FGuid();
	}

	if (!Record.Id.IsValid())
	{
		Record.Id = FGuid::NewGuid();
	}

	// Update if exists
	for (FCharacterPresetRecord& Existing : SaveObject->UserPresets)
	{
		if (Existing.Id == Record.Id)
		{
			Existing = Record;
			SaveUserPresets();
			return Record.Id;
		}
	}

	// Add new
	SaveObject->UserPresets.Add(Record);
	SaveUserPresets();
	return Record.Id;
}

bool UPresetLibrarySubsystem::DeleteUserPresetById(const FGuid& Id)
{
	EnsureSaveObject();
	if (!SaveObject)
	{
		return false;
	}

	const int32 Removed = SaveObject->UserPresets.RemoveAll([&](const FCharacterPresetRecord& R)
	{
		return R.Id == Id;
	});

	if (Removed > 0)
	{
		SaveUserPresets();
		return true;
	}

	return false;
}

bool UPresetLibrarySubsystem::ApplyPresetToMeshById(const FGuid& Id, USkeletalMeshComponent* MeshComp) const
{
	const FCharacterPresetRecord* Record = FindPresetById(Id);
	if (!Record)
	{
		return false;
	}

	Record->ApplyToMeshComponent(MeshComp);
	return true;
}
