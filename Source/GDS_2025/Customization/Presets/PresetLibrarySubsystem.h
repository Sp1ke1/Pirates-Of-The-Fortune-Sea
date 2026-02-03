#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GDS_2025/Customization/Presets/CharacterPresetRecord.h"
#include "PresetLibrarySubsystem.generated.h"

class UPresetPackDataAsset;
class UPlayerPresetSaveGame;
class USkeletalMeshComponent;

UCLASS()
class UPresetLibrarySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// -------------------------
	// Dev presets (content)
	// -------------------------

	// IMPORTANT:
	// UFUNCTION parameters must use raw UObject* (not TObjectPtr)
	UFUNCTION(BlueprintCallable, Category="Presets|Dev")
	void SetDevPresetPacks(const TArray<UPresetPackDataAsset*>& InPacks);

	// -------------------------
	// Query
	// -------------------------

	UFUNCTION(BlueprintCallable, Category="Presets|Query")
	TArray<FCharacterPresetRecord> GetAllPresets() const;

	UFUNCTION(BlueprintCallable, Category="Presets|Query")
	TArray<FGuid> GetAllPresetIds() const;

	// Get only user-created presets (appearances)
	UFUNCTION(BlueprintCallable, Category="Presets|Query")
	TArray<FCharacterPresetRecord> GetUserPresets() const;

	UFUNCTION(BlueprintCallable, Category="Presets|Query")
	TArray<FGuid> GetUserPresetIds() const;

	const FCharacterPresetRecord* FindPresetById(const FGuid& Id) const;

	// -------------------------
	// User presets (saved)
	// -------------------------

	UFUNCTION(BlueprintCallable, Category="Presets|User")
	FGuid AddOrUpdateUserPreset(FCharacterPresetRecord Record);

	UFUNCTION(BlueprintCallable, Category="Presets|User")
	bool DeleteUserPresetById(const FGuid& Id);

	// -------------------------
	// Persistence
	// -------------------------

	UFUNCTION(BlueprintCallable, Category="Presets|Save")
	bool LoadUserPresets();

	UFUNCTION(BlueprintCallable, Category="Presets|Save")
	bool SaveUserPresets();

	// -------------------------
	// Apply
	// -------------------------

	UFUNCTION(BlueprintCallable, Category="Presets|Apply")
	bool ApplyPresetToMeshById(const FGuid& Id, USkeletalMeshComponent* MeshComp) const;

	// Request async preload of all preset meshes (dev + user) so Apply calls are instant later.
	UFUNCTION(BlueprintCallable, Category="Presets|Load")
	void PreloadAllPresetMeshes();

	// -------------------------
	// Settings
	// -------------------------

	UPROPERTY(EditDefaultsOnly, Category="Presets|Save")
	FString SaveSlotName = TEXT("PlayerPresets");

	UPROPERTY(EditDefaultsOnly, Category="Presets|Save")
	int32 SaveUserIndex = 0;

private:
	// INTERNAL STORAGE — TObjectPtr is correct here
	UPROPERTY()
	TArray<TObjectPtr<UPresetPackDataAsset>> DevPacks;

	UPROPERTY()
	TObjectPtr<UPlayerPresetSaveGame> SaveObject;

private:
	void EnsureSaveObject();
	void CollectDevPresets(TArray<FCharacterPresetRecord>& Out) const;

	// Keep handle alive while preloading
	TSharedPtr<struct FStreamableHandle> PreloadHandle;
};

