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

	TeamIndex = 0;
	TeamZoneTag = "Zone_Team0";
	EnemyBaseTag = "Base_Team1";
	TargetItemTag = "Pickable";
	InteractionRange = 150.0f;
	CurrentState = EBotState::Idle;
	WaitTimer = 0.0f;
	bDebugDraw = false;
}


// Called when the game starts
void UBotLogicComponent::BeginPlay()
{
	Super::BeginPlay();

	// Apply team tags based on initial TeamIndex
	SetTeamIndex(TeamIndex);

	// Randomize start time slightly to avoid all bots starting exactly same frame
	WaitTimer = FMath::RandRange(0.0f, 1.0f);
}

void UBotLogicComponent::SetTeamIndex(int32 NewTeamIndex)
{
	TeamIndex = NewTeamIndex;
	TeamZoneTag = FName(*FString::Printf(TEXT("Zone_Team%d"), TeamIndex));
	const int32 EnemyTeamIndex = TeamIndex == 1 ? 0 : 1;
	EnemyBaseTag = FName(*FString::Printf(TEXT("Base_Team%d"), EnemyTeamIndex));
}


// Called every frame
void UBotLogicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (!IsActive())
	{
		return;
	}
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
			// Look for item inside this bot's zone
			AActor* FoundItem = FindNearestItemInZone();
			if (FoundItem)
			{
				CurrentTargetActor = FoundItem;
				if (MoveToActor(CurrentTargetActor))
				{
					CurrentState = EBotState::MovingToItem;
				}
				else
				{
					CurrentTargetActor = nullptr;
					WaitTimer = 1.0f;
				}
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
			else
			{
				// Check that the item is still inside our zone
				if (!IsActorInTeamZone(CurrentTargetActor))
				{
					// Item left the zone -- abort and look for another item
					CurrentTargetActor = nullptr;
					if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
					{
						if (AAIController* AICon = Cast<AAIController>(OwnerPawn->GetController()))
						{
							AICon->StopMovement();
						}
					}
					CurrentState = EBotState::Idle;
				}
				else
				{
					if (!MoveToActor(CurrentTargetActor))
					{
						CurrentState = EBotState::Idle;
						CurrentTargetActor = nullptr;
						WaitTimer = 1.0f;
					}
				}
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

			// Look for enemy base
			AActor* FoundBase = FindRandomActorWithTag(EnemyBaseTag);
			if (FoundBase)
			{
				CurrentTargetActor = FoundBase;
				if (MoveToActor(CurrentTargetActor))
				{
					CurrentState = EBotState::MovingToBase;
				}
				else
				{
					CurrentTargetActor = nullptr;
					WaitTimer = 1.0f;
				}
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
			float DistSq = FVector::DistSquared(MyLoc, GetActorPhysicalLocation(Actor));
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

bool UBotLogicComponent::IsActorInTeamZone(AActor* Actor) const
{
	if (!Actor) return false;

	TArray<AActor*> ZoneActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TeamZoneTag, ZoneActors);
	if (ZoneActors.Num() == 0) return true; // No zone defined - assume valid

	TArray<AActor*> OverlappingActors;
	ZoneActors[0]->GetOverlappingActors(OverlappingActors);
	return OverlappingActors.Contains(Actor);
}

AActor* UBotLogicComponent::FindNearestItemInZone()
{
	// Find the zone actor with our team's zone tag
	AActor* ZoneActor = nullptr;
	{
		TArray<AActor*> ZoneActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), TeamZoneTag, ZoneActors);
		if (ZoneActors.Num() > 0)
		{
			ZoneActor = ZoneActors[0]; // Expect one zone per team
		}
	}

	if (!ZoneActor)
	{
		// No zone found - fall back to global nearest item
		UE_LOG(LogTemp, Warning, TEXT("BotLogicComponent: No zone actor found with tag '%s', falling back to global search"), *TeamZoneTag.ToString());
		return FindNearestActorWithTag(TargetItemTag);
	}

	// Collect all items and filter by whether they overlap the zone
	TArray<AActor*> AllItems;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetItemTag, AllItems);

	AActor* Nearest = nullptr;
	float MinDistSq = FLT_MAX;
	FVector MyLoc = GetOwner()->GetActorLocation();

	for (AActor* Item : AllItems)
	{
		if (!Item || Item->IsAttachedTo(GetOwner())) continue;

		// Check if the item is overlapping / inside the zone volume
		TArray<AActor*> OverlappingActors;
		ZoneActor->GetOverlappingActors(OverlappingActors);

		if (OverlappingActors.Contains(Item))
		{
			float DistSq = FVector::DistSquared(MyLoc, GetActorPhysicalLocation(Item));
			if (DistSq < MinDistSq)
			{
				MinDistSq = DistSq;
				Nearest = Item;
			}
		}
	}
	return Nearest;
}

bool UBotLogicComponent::MoveToActor(AActor* Target)
{
	if (!Target) return false;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return false;

	AAIController* AICon = Cast<AAIController>(OwnerPawn->GetController());
	if (AICon)
	{
		FVector GoalLocation = GetActorPhysicalLocation(Target);
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

		// Use MoveToLocation to ensure we go to the valid NavMesh point
		FAIMoveRequest MoveReq(GoalLocation);
		MoveReq.SetUsePathfinding(true);
		MoveReq.SetAcceptanceRadius(50.0f);
		MoveReq.SetAllowPartialPath(true);
		MoveReq.SetGoalLocation(GoalLocation);

		FPathFollowingRequestResult Result = AICon->MoveTo(MoveReq);
		return Result.Code != EPathFollowingRequestResult::Failed;
	}
	return false;
}

FVector UBotLogicComponent::GetActorPhysicalLocation(AActor* Target)
{
	if (!Target)
	{
		return FVector();
	}
	if (UStaticMeshComponent* StaticMesh = Target->GetComponentByClass<UStaticMeshComponent>())
	{
		return StaticMesh->GetComponentLocation();
	}
	return Target->GetActorLocation();
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
	
	float Dist = FVector::Dist(GetOwner()->GetActorLocation(), GetActorPhysicalLocation(Target));
	return Dist <= InteractionRange;
}
