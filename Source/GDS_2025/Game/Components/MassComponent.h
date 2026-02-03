// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MassComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GDS_2025_API UMassComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMassComponent();

	UFUNCTION(BlueprintNativeEvent)
	float GetMass();
	
protected: 
	UPROPERTY(EditAnywhere) 
	float Mass = 1.f; 
};
