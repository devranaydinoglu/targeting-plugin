// Copyright 2023 devran All Rights Reserved.

#include "TargetingComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"

// Sets default values for this component's properties
UTargetingComponent::UTargetingComponent()
	: PlayerCharacter(nullptr), PlayerCamera(nullptr), bStartTimer(true), SearchRadius(0.0f), SearchInterval(0.0f), MaxHorizontalCameraAngle(0.0f), 
	MaxVerticalCameraAngle(0.0f), MaxHorizontalPlayerHalfAngle(0.0f), CameraDirectionMultiplier(0.0f), DistanceMultiplier(0.0f), PlayerDirectionMultiplier(0.0f), 
	TargetTag(""), TargetClass(nullptr), TargetTraceChannel(ETraceTypeQuery::TraceTypeQuery1), BlockingTraceChannel(ETraceTypeQuery::TraceTypeQuery1), 
	Target(nullptr), bDebug(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
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

// Initialize component and activate targeting
void UTargetingComponent::Initialize(ACharacter* InPlayerCharacter, UCameraComponent* InPlayerCamera)
{
	#if !UE_BUILD_SHIPPING
	if (bDebug)
	{
		if (!IsValid(InPlayerCharacter))
		{
			UE_LOG(LogTemp, Warning, TEXT("Player Character has not been set on Initialize!"));
		}
		
		if (!IsValid(InPlayerCamera))
		{
			UE_LOG(LogTemp, Warning, TEXT("Player Camera has not been set on Initialize!"));
		}
	}
	#endif // !UE_BUILD_SHIPPING
	// Set references
	PlayerCharacter = InPlayerCharacter;
	PlayerCamera = InPlayerCamera;

	// Start the search timer
	if (IsValid(PlayerCharacter))
		ActivateTargeting();
}

// Start or unpause search timer
void UTargetingComponent::ActivateTargeting()
{
	if (IsValid(PlayerCharacter))
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
	else
	{
		#if !UE_BUILD_SHIPPING
		if (bDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("Player Character is not valid!"));
		}
		#endif // !UE_BUILD_SHIPPING
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

	UKismetSystemLibrary::SphereTraceMulti(this, StartEnd, StartEnd, SearchRadius, TargetTraceChannel, false, ActorsToIgnore, EDrawDebugTrace::None, Hits, true);

	TArray<AActor*> Targets;

	for (const FHitResult& HitItr : Hits)
	{
		// Check if actor is in player's vision
		bool bInVision = true;
		if (IsValid(PlayerCamera)) bInVision = IsInVision(HitItr.GetActor());

		// Check if nothing is blocking vision to target
		FVector Start = PlayerCharacter->GetActorLocation();
		FVector End = HitItr.GetActor()->GetActorLocation();
		FHitResult Hit;

		bool bHit = UKismetSystemLibrary::LineTraceSingle(this, Start, End, BlockingTraceChannel, false, ActorsToIgnore, EDrawDebugTrace::None, Hit, true);

		if ((HitItr.GetActor()->ActorHasTag(TargetTag) || HitItr.GetActor()->IsA(TargetClass)) && bInVision && !bHit)
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
bool UTargetingComponent::IsInVision(const AActor* Actor)
{
	if (IsValid(PlayerCamera))
	{
		FRotator CameraLookAtRotation = UKismetMathLibrary::FindLookAtRotation(PlayerCamera->GetComponentLocation(), Actor->GetActorLocation());
		FRotator CameraActorDelta = UKismetMathLibrary::NormalizedDeltaRotator(CameraLookAtRotation, PlayerCharacter->GetControlRotation());

		FRotator CharacterLookAtRotation = UKismetMathLibrary::FindLookAtRotation(PlayerCharacter->GetActorLocation(), Actor->GetActorLocation());
		FRotator CharacterActorDelta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterLookAtRotation, PlayerCharacter->GetActorRotation());

		bool bInCameraVision = UKismetMathLibrary::Abs(CameraActorDelta.Pitch) <= MaxVerticalCameraAngle && UKismetMathLibrary::Abs(CameraActorDelta.Yaw) <= MaxHorizontalCameraAngle;
		bool bInCharacterVision = UKismetMathLibrary::Abs(CharacterActorDelta.Yaw) <= MaxHorizontalPlayerHalfAngle;
		
		// Check if target is within the vertical and horizontal vision
		if (bInCameraVision && bInCharacterVision)
		{
			return true;
		}

		return false;
	}
	else
	{
		#if !UE_BUILD_SHIPPING
		if (bDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("Player Camera is not valid!"));
		}
		#endif // !UE_BUILD_SHIPPING
		return true;
	}
}

// Find most optimal target
AActor* UTargetingComponent::FindOptimalTarget(TArray<AActor*> TargetsArray)
{
	RankedTargets.Empty();

	if (TargetsArray.IsEmpty())
	{
		return nullptr;
	}

	for (AActor* TargetIt : TargetsArray)
	{
		FTargetData TargetData;
		TargetData.Target = TargetIt;
		TargetData.Score = ScoreTarget(TargetIt);

		RankedTargets.Add(TargetData);
	}

	RankedTargets.Sort([](const FTargetData& a, const FTargetData& b) {return a.Score > b.Score;});

	return RankedTargets[0].Target;
}

// Score target on each criteria
float UTargetingComponent::ScoreTarget(const AActor* TargetToScore)
{
	if (IsValid(TargetToScore))
	{
		// Angle to camera direction
		float CameraDirectionScore = 0.0f;

		if (IsValid(PlayerCamera))
		{
			FVector CameraTargetUnitDirection = (TargetToScore->GetActorLocation() - PlayerCamera->GetComponentLocation()).GetSafeNormal();
			float CameraTargetDotProduct = FVector::DotProduct(PlayerCamera->GetForwardVector(), CameraTargetUnitDirection);
			float CameraTargetDegrees = FMath::Acos(CameraTargetDotProduct) * 180.0 / PI;

			CameraDirectionScore = UKismetMathLibrary::MapRangeClamped(CameraTargetDegrees, 45.0, 0.0, 1.0, 10.0) * CameraDirectionMultiplier;
		}
		else
		{
			#if !UE_BUILD_SHIPPING
			if (bDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("Player Camera is not valid!"));
			}
			#endif // !UE_BUILD_SHIPPING
		}

		// Distance to player character
		float DistanceScore = 0.0f;

		// Angle to player character direction
		float PlayerDirectionScore = 0.0f;

		if (IsValid(PlayerCharacter))
		{
			// Distance to player character
			float Distance = PlayerCharacter->GetDistanceTo(TargetToScore);

			DistanceScore = UKismetMathLibrary::MapRangeClamped(Distance, 0.0, SearchRadius, 10.0, 1.0) * DistanceMultiplier;

			// Angle to player character direction
			FVector PlayerTargetUnitDirection = (TargetToScore->GetActorLocation() - PlayerCharacter->GetActorLocation()).GetSafeNormal();
			float PlayerTargetDotProduct = FVector::DotProduct(PlayerCharacter->GetActorForwardVector(), PlayerTargetUnitDirection);
			
			PlayerDirectionScore = UKismetMathLibrary::MapRangeClamped(PlayerTargetDotProduct, 0.0, 1.0, 1.0, 10.0) * PlayerDirectionMultiplier;
		}
		else
		{
			#if !UE_BUILD_SHIPPING
			if (bDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("Player Character is not valid!"));
			}
			#endif // !UE_BUILD_SHIPPING
		}

		// Calculate final score
		float FinalScore = CameraDirectionScore + DistanceScore + PlayerDirectionScore;

		return FinalScore;
	}
	else
	{
		return 0.0f;
	}
}
