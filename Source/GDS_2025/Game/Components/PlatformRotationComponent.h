// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlatformRotationComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnMaxAngleReachedSignature, bool, bSign ); 

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GDS_2025_API UPlatformRotationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlatformRotationComponent();
	virtual void InitializeComponent() override;
	
	UFUNCTION(BlueprintCallable) 
	void ResetRotation();
	
	UFUNCTION(BlueprintCallable)
	void AddDeltaRotation(float InDeltaRotation);
	
	UPROPERTY(BlueprintAssignable) 
	FOnMaxAngleReachedSignature	OnMaxAngleReachedDelegate;
	
	UFUNCTION(BlueprintCallable)
	float GetRotationSpeed() const;
	
protected:
	
	UPROPERTY(EditAnywhere) 
	FName RotationRootComponentName = FName("RotationRoot");
	
	UPROPERTY(EditAnywhere)
	float MaxRotation = 30.f;
	
	UPROPERTY(EditAnywhere)
	float RotationSpeed = 3.f;
	
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EAxis::Type> RotationAxis = EAxis::Type::X;
	
	UPROPERTY() 
	USceneComponent* RotationRoot; 
};
