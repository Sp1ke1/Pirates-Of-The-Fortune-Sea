#include "GDS_2025/Customization/Components/CharacterPresetApplierComponent.h"
#include "GDS_2025/Customization/Presets/PresetLibrarySubsystem.h"
#include "GDS_2025/Lobby/Framework/LobbyGameInstance.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "UObject/SoftObjectPath.h"
#include "Materials/MaterialInterface.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"

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

void UCharacterPresetApplierComponent::Initialize(USkeletalMeshComponent* InMainMeshComponent, UStaticMeshComponent* InHatMeshComponent, UStaticMeshComponent* InEarringMeshComponent, UStaticMeshComponent* InPipeMeshComponent, UStaticMeshComponent* InGlassesMeshComponent)
{
	MainMeshComponent = InMainMeshComponent;
	HatMeshComponent = InHatMeshComponent;
	EarringMeshComponent = InEarringMeshComponent;
	PipeMeshComponent = InPipeMeshComponent;
	GlassesMeshComponent = InGlassesMeshComponent;
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

bool UCharacterPresetApplierComponent::ApplyAppearance(int32 SlotIndex)
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI)
	{
		return false;
	}

	ULobbyGameInstance* LobbyGI = Cast<ULobbyGameInstance>(GI);
	if (!LobbyGI)
	{
		return false;
	}

	// Проверка валидности индекса слота
	const TArray<FLobbySlotData>& Slots = LobbyGI->GetAllSlots();
	if (!Slots.IsValidIndex(SlotIndex))
	{
		return false;
	}

	const FLobbySlotData& SlotData = LobbyGI->GetSlotData(SlotIndex);
	return ApplyPresetById(SlotData.SelectedPresetId);
}

bool UCharacterPresetApplierComponent::ApplyRandomPreset()
{
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

	TArray<FGuid> AllIds = Lib->GetAllPresetIds();
	if (AllIds.IsEmpty())
	{
		return false;
	}

	int32 RandomIndex = FMath::RandRange(0, AllIds.Num() - 1);
	return ApplyPresetById(AllIds[RandomIndex]);
}

bool UCharacterPresetApplierComponent::ApplyFirstPredefinedPreset()
{
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

	TArray<FGuid> AllIds = Lib->GetAllPresetIds();
	TArray<FGuid> UserIds = Lib->GetUserPresetIds();

	for (const FGuid& Id : AllIds)
	{
		if (!UserIds.Contains(Id))
		{
			return ApplyPresetById(Id);
		}
	}

	return false;
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

void UCharacterPresetApplierComponent::ApplyItem_Implementation(const FCustomizationSlotItem& Item, int32 SlotIndex)
{
	// Default C++ implementation handles basic cases
	// Override in Blueprint for full customization logic
	
	switch (Item.SlotType)
	{
	case ECustomizationSlotType::MainMesh:
		// Apply skeletal mesh to MainMeshComponent
		if (MainMeshComponent && Item.Mesh.IsValid())
		{
			const FSoftObjectPath MeshPath = Item.Mesh.ToSoftObjectPath();
			
			if (USkeletalMesh* AlreadyLoaded = Item.Mesh.Get())
			{
				MainMeshComponent->SetSkeletalMesh(AlreadyLoaded);
			}
			else if (MeshPath.IsValid())
			{
				TWeakObjectPtr<USkeletalMeshComponent> WeakMeshComp = MainMeshComponent;
				FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
				Streamable.RequestAsyncLoad(MeshPath, FStreamableDelegate::CreateLambda([WeakMeshComp, MeshPath]()
				{
					if (!WeakMeshComp.IsValid())
					{
						return;
					}

					UObject* LoadedObject = MeshPath.ResolveObject();
					if (USkeletalMesh* LoadedMesh = Cast<USkeletalMesh>(LoadedObject))
					{
						if (USkeletalMeshComponent* MeshCompPtr = WeakMeshComp.Get())
						{
							MeshCompPtr->SetSkeletalMesh(LoadedMesh);
						}
					}
				}));
			}
		}
		break;

	case ECustomizationSlotType::Material:
		// Apply material to MainMeshComponent
		if (MainMeshComponent && Item.Material.IsValid())
		{
			const FSoftObjectPath MaterialPath = Item.Material.ToSoftObjectPath();
			
			if (UMaterialInterface* AlreadyLoaded = Item.Material.Get())
			{
				MainMeshComponent->SetMaterial(0, AlreadyLoaded);
			}
			else if (MaterialPath.IsValid())
			{
				TWeakObjectPtr<USkeletalMeshComponent> WeakMeshComp = MainMeshComponent;
				FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
				Streamable.RequestAsyncLoad(MaterialPath, FStreamableDelegate::CreateLambda([WeakMeshComp, MaterialPath]()
				{
					if (!WeakMeshComp.IsValid())
					{
						return;
					}

					UObject* LoadedObject = MaterialPath.ResolveObject();
					if (UMaterialInterface* LoadedMaterial = Cast<UMaterialInterface>(LoadedObject))
					{
						if (USkeletalMeshComponent* MeshCompPtr = WeakMeshComp.Get())
						{
							MeshCompPtr->SetMaterial(0, LoadedMaterial);
						}
					}
				}));
			}
		}
		break;

	case ECustomizationSlotType::Hat:
	case ECustomizationSlotType::Earring:
		// Default C++ implementation doesn't handle these
		// Override in Blueprint to handle HatMeshComponent, StaticMesh, etc.
		break;
	}
}

