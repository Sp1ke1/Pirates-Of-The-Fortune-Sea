#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GDS_2025/Lobby/Data/LobbySlotData.h"
#include "LobbySlotActor.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class UTextRenderComponent;
class UStaticMeshComponent;
class USkeletalMesh;


UCLASS()
class ALobbySlotActor : public AActor
{
	GENERATED_BODY()

public:
	ALobbySlotActor();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lobby")
	int32 SlotIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<USceneComponent> Root = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<USceneComponent> FocusAnchor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<USkeletalMeshComponent> CharacterMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<UTextRenderComponent> ChangeText = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<UStaticMeshComponent> LeftTriangle = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<UStaticMeshComponent> RightTriangle = nullptr;

	UFUNCTION(BlueprintCallable, Category="Lobby")
	void ApplyData(const FLobbySlotData& Data);

	// Mouse hover feedback (visual)
	UFUNCTION(BlueprintCallable, Category="Lobby|Focus")
	void SetHovered(bool bHovered);

	// Active focus (gamepad/keyboard/etc). Multiple controllers may focus the same slot.
	UFUNCTION(BlueprintCallable, Category="Lobby|Focus")
	void AddActiveFocus();

	UFUNCTION(BlueprintCallable, Category="Lobby|Focus")
	void RemoveActiveFocus();

	UFUNCTION(BlueprintCallable, Category="Lobby|Focus")
	bool HasActiveFocus() const { return ActiveFocusCount > 0; }

	UFUNCTION(BlueprintCallable, Category="Lobby")
	USceneComponent* GetFocusAnchor() const { return FocusAnchor; }

	// Click handlers for triangles
	UFUNCTION()
	void OnLeftTriangleClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void OnRightTriangleClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	// Presets (for now just skeletal meshes). Set these in BP_LobbySlotActor.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby|Presets")
	TArray<TObjectPtr<USkeletalMesh>> PresetMeshes;

private:
	UPROPERTY()
	bool bIsHovered = false;

	UPROPERTY()
	int32 ActiveFocusCount = 0;

	// Tuning
	UPROPERTY(EditAnywhere, Category="Lobby|Focus")
	float HoverScale = 1.05f;

	UPROPERTY(EditAnywhere, Category="Lobby|Focus")
	float ActiveScale = 1.08f;


	void UpdateFocusVisuals();
};
