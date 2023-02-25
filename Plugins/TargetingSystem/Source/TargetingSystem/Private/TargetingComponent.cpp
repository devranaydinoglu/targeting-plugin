// Fill out your copyright notice in the Description page of Project Settings.

#include "TargetingComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"

// Sets default values for this component's properties
UTargetingComponent::UTargetingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bStartTimer = true;
}

// Called when the game starts
void UTargetingComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


// Initialize component
void UTargetingComponent::Initialize(ACharacter* Character, UCameraComponent* Camera)
{
	// Set references
	PlayerCharacter = Character;
	PlayerCamera = Camera;

	// Start the search timer
	ActivateTargeting();
}

// Start or unpause search timer
void UTargetingComponent::ActivateTargeting()
{
	// If the timer is not active yet, start it. Else unpause the timer
	if (!bStartTimer)
	{
		GetWorld()->GetTimerManager().UnPauseTimer(SearchTimerHandle);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(SearchTimerHandle, this, &UTargetingComponent::SetTarget, SearchInterval, true);
		bStartTimer = false;
	}
}

// Pause search timer
void UTargetingComponent::DeactivateTargeting()
{
	GetWorld()->GetTimerManager().PauseTimer(SearchTimerHandle);
}

// Set optimal target as target
void UTargetingComponent::SetTarget()
{
	FVector StartEnd = PlayerCharacter->GetActorLocation();
	TArray<AActor*> ActorsToIgnore;
	TArray<FHitResult> Hits;

	UKismetSystemLibrary::SphereTraceMulti(GetWorld(), StartEnd, StartEnd, SearchRadius, TargetTraceChannel, false, ActorsToIgnore, EDrawDebugTrace::None, Hits, true);

	TArray<AActor*> Targets;

	for (const FHitResult& HitItr : Hits)
	{
		// Check if actor is in player's vision
		bool bInVision = IsInVision(HitItr.GetActor());

		// Check if nothing is blocking vision to target
		FVector Start = PlayerCharacter->GetActorLocation();
		FVector End = HitItr.GetActor()->GetActorLocation();
		FHitResult Hit;

		bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), Start, End, BlockingTraceChannel, false, ActorsToIgnore, EDrawDebugTrace::None, Hit, true);

		if (HitItr.GetActor()->ActorHasTag(TargetTag) && bInVision && !bHit)
		{
			Targets.Add(HitItr.GetActor());
		}
	}

	// Find optimal target and set as target
	AActor* TempTarget = FindOptimalTarget(Targets);

	if (IsValid(TempTarget))
	{
		Target = TempTarget;
	}
	else
	{
		Target = nullptr;
	}
}

// Check if actor is in player's vision
bool UTargetingComponent::IsInVision(AActor* Actor)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(PlayerCamera->GetComponentLocation(), Actor->GetActorLocation());
	FRotator ActorDelta = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRotation, PlayerCharacter->GetControlRotation());

	// Check if target is within the vertical and horizontal vision
	if (UKismetMathLibrary::Abs(ActorDelta.Pitch) <= MaxVerticalVisionAngle && UKismetMathLibrary::Abs(ActorDelta.Yaw) <= MaxHorizontalVisionAngle)
	{
		return true;
	}

	return false;
}

// Find most optimal target
AActor* UTargetingComponent::FindOptimalTarget(TArray<AActor*> TargetsArray)
{
	if (TargetsArray.Num() > 0)
	{
		TArray<float> TargetScores;
		TargetScores.Empty();

		for (AActor* TempTarget : TargetsArray)
		{
			TargetScores.Add(ScoreTarget(TempTarget));
		}

		int32 IndexOfMaxValue;
		float MaxValue;

		UKismetMathLibrary::MaxOfFloatArray(TargetScores, IndexOfMaxValue, MaxValue);

		return TargetsArray[IndexOfMaxValue];
	}
	else
	{
		return nullptr;
	}
}

// Score target on each criteria
float UTargetingComponent::ScoreTarget(AActor* TargetToScore)
{
	if (IsValid(TargetToScore))
	{
		// Angle to camera direction
		FVector CameraTargetUnitDirection = UKismetMathLibrary::GetDirectionUnitVector(PlayerCamera->GetComponentLocation(), TargetToScore->GetActorLocation());
		float CameraTargetDotProduct = UKismetMathLibrary::Dot_VectorVector(PlayerCamera->GetForwardVector(), CameraTargetUnitDirection);

		float CameraDirectionScore = UKismetMathLibrary::MapRangeClamped(CameraTargetDotProduct, 0.0, 1.0, 1.0, 10.0) * CameraDirectionMultiplier;

		// Distance to player character
		float Distance = PlayerCharacter->GetDistanceTo(TargetToScore);

		float DistanceScore = UKismetMathLibrary::MapRangeClamped(Distance, 0.0, SearchRadius, 10.0, 1.0) * DistanceMultiplier;

		// Angle to player character direction
		FVector PlayerTargetUnitDirection = UKismetMathLibrary::GetDirectionUnitVector(PlayerCharacter->GetActorLocation(), TargetToScore->GetActorLocation());
		float PlayerTargetDotProduct = UKismetMathLibrary::Dot_VectorVector(PlayerCharacter->GetActorForwardVector(), PlayerTargetUnitDirection);

		float PlayerDirectionScore = UKismetMathLibrary::MapRangeClamped(PlayerTargetDotProduct, 0.0, 1.0, 1.0, 10.0) * PlayerDirectionMultiplier;

		// Calculate final score
		float FinalScore = CameraDirectionScore + DistanceScore + PlayerDirectionScore;

		return FinalScore;
	}
	else
	{
		return 0.0f;
	}
}
