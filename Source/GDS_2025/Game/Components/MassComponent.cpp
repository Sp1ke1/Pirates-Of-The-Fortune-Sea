#include "MassComponent.h"

UMassComponent::UMassComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

float UMassComponent::GetMass_Implementation()
{
	return Mass; 
}

