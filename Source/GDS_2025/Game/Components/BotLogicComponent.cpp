#include "BotLogicComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "GDS_2025/Game/Interfaces/PickupInterface.h"

// Sets default values for this component's properties
UBotLogicComponent::UBotLogicComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	TargetItemTag = "Pickable";
	TargetBaseTag = "EnemyBase";
	InteractionRange = 150.0f;
	CurrentState = EBotState::Idle;
	WaitTimer = 0.0f;
}


// Called when the game starts
void UBotLogicComponent::BeginPlay()
{
	Super::BeginPlay();

	// Randomize start time slightly to avoid all bots starting exactly same frame
	WaitTimer = FMath::RandRange(0.0f, 1.0f);
}


// Called every frame
void UBotLogicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateLogic(DeltaTime);
}

void UBotLogicComponent::UpdateLogic(float DeltaTime)
{
	if (WaitTimer > 0.0f)
	{
		WaitTimer -= DeltaTime;
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;

	switch (CurrentState)
	{
	case EBotState::Idle:
		{
			// Look for item
			AActor* FoundItem = FindNearestActorWithTag(TargetItemTag);
			if (FoundItem)
			{
				CurrentTargetActor = FoundItem;
				CurrentState = EBotState::MovingToItem;
				MoveToActor(CurrentTargetActor);
			}
			else
			{
				WaitTimer = 1.0f; // Wait before searching again
			}
		}
		break;

	case EBotState::MovingToItem:
		{
			if (CurrentTargetActor && IsTargetReached(CurrentTargetActor))
			{
				IPickupInterface* PickupComp = GetPickupComponent();
				//if (PickupComp && PickupComp->AttemptPickup(CurrentTargetActor)) // TODO uncomment after Pickupcomponent implementation
				{
					CarriedItem = CurrentTargetActor;
					CurrentState = EBotState::CarryingItem;
					WaitTimer = 0.5f; // Small delay
				}
				//else
				//{
				//	WaitTimer = 0.5f; // Retry later
				//}
			}
			else if (!CurrentTargetActor)
			{
				CurrentState = EBotState::Idle;
			}
		}
		break;

	case EBotState::CarryingItem:
		{
			// Verify we still have item
			if (!CarriedItem)
			{
				CurrentState = EBotState::Idle;
				return;
			}

			// Look for base
			AActor* FoundBase = FindRandomActorWithTag(TargetBaseTag);
			if (FoundBase)
			{
				CurrentTargetActor = FoundBase;
				CurrentState = EBotState::MovingToBase;
				MoveToActor(CurrentTargetActor);
			}
			else
			{
				// No base found? Stay here? Or wander?
				WaitTimer = 1.0f;
			}
		}
		break;

	case EBotState::MovingToBase:
		{
			if (!CarriedItem)
			{
				CurrentState = EBotState::Idle;
				return;
			}

			if (CurrentTargetActor && IsTargetReached(CurrentTargetActor))
			{
				IPickupInterface* PickupComp = GetPickupComponent();
				if (PickupComp)
				{
					PickupComp->AttemptDrop();
				}
				CarriedItem = nullptr;
				CurrentState = EBotState::Idle;
				WaitTimer = 1.0f;
			}
			else if (!CurrentTargetActor)
			{
				// Base destroyed?
				CurrentState = EBotState::CarryingItem; // Go back to looking for base
			}
		}
		break;
	}
}

AActor* UBotLogicComponent::FindNearestActorWithTag(FName Tag)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, FoundActors);

	AActor* Nearest = nullptr;
	float MinDistSq = FLT_MAX;
	FVector MyLoc = GetOwner()->GetActorLocation();

	for (AActor* Actor : FoundActors)
	{
		if (Actor && !Actor->IsAttachedTo(GetOwner())) // Don't find what we already hold (if tags persist)
		{
			float DistSq = FVector::DistSquared(MyLoc, Actor->GetActorLocation());
			if (DistSq < MinDistSq)
			{
				MinDistSq = DistSq;
				Nearest = Actor;
			}
		}
	}
	return Nearest;
}

AActor* UBotLogicComponent::FindRandomActorWithTag(FName Tag)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, FoundActors);

	if (FoundActors.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, FoundActors.Num() - 1);
		return FoundActors[Index];
	}
	return nullptr;
}

void UBotLogicComponent::MoveToActor(AActor* Target)
{
	if (!Target) return;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	AAIController* AICon = Cast<AAIController>(OwnerPawn->GetController());
	if (AICon)
	{
		FVector GoalLocation = Target->GetActorLocation();
		bool bProjected = false;

		// Explicitly project to NavMesh to handle objects cutting the mesh
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (NavSys)
		{
			FNavLocation ValidLocation;
			// Search radius: usually interaction range is a good start. 
			// If the hole is HUGE, this might need to be bigger.
			FVector QueryExtent = FVector(InteractionRange * 2.0f, InteractionRange * 2.0f, 500.0f);
			
			if (NavSys->ProjectPointToNavigation(GoalLocation, ValidLocation, QueryExtent))
			{
				GoalLocation = ValidLocation.Location;
				bProjected = true;
			}
		}

		// Debug visualization
		if (bDebugDraw)
		{
			if (bProjected)
			{
				DrawDebugSphere(GetWorld(), GoalLocation, 20.0f, 12, FColor::Green, false, 1.0f);
				DrawDebugLine(GetWorld(), Target->GetActorLocation(), GoalLocation, FColor::Green, false, 1.0f);
			}
			else
			{
				DrawDebugSphere(GetWorld(), GoalLocation, 20.0f, 12, FColor::Red, false, 1.0f);
			}
		}

		// Use MoveToLocation instead of MoveToActor to ensure we go to the VALID point we found
		FAIMoveRequest MoveReq(GoalLocation);
		MoveReq.SetUsePathfinding(true);
		MoveReq.SetAcceptanceRadius(50.0f); // We are already targeting the edge, so we want to get quite close to it
		MoveReq.SetAllowPartialPath(true);
		
		AICon->MoveTo(MoveReq);
	}
}

IPickupInterface* UBotLogicComponent::GetPickupComponent() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	TSet<UActorComponent*> Components = Owner->GetComponents();
	for (UActorComponent* Comp : Components)
	{
		if (Comp && Comp->Implements<UPickupInterface>())
		{
			return Cast<IPickupInterface>(Comp);
		}
	}
	
	if (Owner->Implements<UPickupInterface>())
	{
		return Cast<IPickupInterface>(Owner);
	}

	return nullptr;
}

bool UBotLogicComponent::IsTargetReached(AActor* Target)
{
	if (!Target) return false;
	
	float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
	return Dist <= InteractionRange;
}
