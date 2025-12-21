#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "LobbyFocusLampActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class ALobbyFocusLampActor : public AActor
{
	GENERATED_BODY()

public:
	ALobbyFocusLampActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<USceneComponent> Root = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<UStaticMeshComponent> LampMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Components")
	TObjectPtr<UPointLightComponent> Light = nullptr;

	// Attach/move to a slot's anchor
	UFUNCTION(BlueprintCallable, Category="Lobby|Focus")
	void AttachToAnchor(USceneComponent* Anchor);

	// Set lamp color (mesh material is up to you; light color will change)
	UFUNCTION(BlueprintCallable, Category="Lobby|Focus")
	void SetColor(const FLinearColor& Color);

	// Optional highlight
	UFUNCTION(BlueprintCallable, Category="Lobby|Focus")
	void SetVisibleLamp(bool bVisible);

private:
	UPROPERTY()
	TWeakObjectPtr<USceneComponent> CurrentAnchor;
};
