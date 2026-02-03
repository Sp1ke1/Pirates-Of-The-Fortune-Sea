#include "GDS_2025/Customization/Framework/CustomizationGameMode.h"
#include "GDS_2025/Customization/Actors/CustomizationPreviewActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"

ACustomizationGameMode::ACustomizationGameMode()
{
}

void ACustomizationGameMode::BeginPlay()
{
	Super::BeginPlay();

	FindPreviewActor();
	BP_OpenMainMenuWidget();
}

void ACustomizationGameMode::FindPreviewActor()
{
	if (!GetWorld())
	{
		return;
	}

	// Find the first ACustomizationPreviewActor in the level
	for (TActorIterator<ACustomizationPreviewActor> It(GetWorld()); It; ++It)
	{
		PreviewActor = *It;
		break;
	}

	if (!PreviewActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CustomizationGameMode] No ACustomizationPreviewActor found in level. Place one in the map."));
	}
}

