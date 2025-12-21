#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GDS_2025/Lobby/Data/LobbySlotData.h"

#include "LobbySlotActor.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class UTextRenderComponent;
class UStaticMeshComponent;

UCLASS()
class ALobbySlotActor : public AActor
{
	GENERATED_BODY()

public:
	ALobbySlotActor();

	// Which slot this actor represents (0..3)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lobby")
	int32 SlotIndex = 0;

	// Root
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<USceneComponent> Root = nullptr;

	// Where focus lamp should attach / move to
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<USceneComponent> FocusAnchor = nullptr;

	// Character mesh (visual)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<USkeletalMeshComponent> CharacterMesh = nullptr;

	// "Change" text button in world
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<UTextRenderComponent> ChangeText = nullptr;

	// Triangles (optional visuals, can be clicked later)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<UStaticMeshComponent> LeftTriangle = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<UStaticMeshComponent> RightTriangle = nullptr;

	// Apply persistent slot data to the visuals
	UFUNCTION(BlueprintCallable, Category="Lobby")
	void ApplyData(const FLobbySlotData& Data);

	// Hover feedback from mouse focus (pure visual)
	UFUNCTION(BlueprintCallable, Category="Lobby")
	void SetHovered(bool bHovered);

	// For convenience
	UFUNCTION(BlueprintCallable, Category="Lobby")
	USceneComponent* GetFocusAnchor() const { return FocusAnchor; }

private:
	UPROPERTY()
	bool bIsHovered = false;

	void UpdateHoverVisuals();
};
