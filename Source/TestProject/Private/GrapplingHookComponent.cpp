#include "GrapplingHookComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UGrapplingHookComponent::UGrapplingHookComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UGrapplingHookComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UGrapplingHookComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsGrappling || !OwnerCharacter)
        return;

    FVector CurrentLocation = OwnerCharacter->GetActorLocation();
    FVector Direction = (GrappleTargetPoint - CurrentLocation);
    float Distance = Direction.Size();

    // Auto-release when close enough
    if (Distance <= ArrivalRadius)
    {
        ReleaseHook();
        return;
    }

    // Apply velocity towards grapple point each tick (overrides gravity during pull)
    Direction.Normalize();
    FVector PullVelocity = Direction * PullSpeed;

    UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
    if (Movement)
    {
        Movement->Velocity = PullVelocity;
    }

#if WITH_EDITOR
    DrawDebugLine(GetWorld(), CurrentLocation, GrappleTargetPoint, FColor::Yellow, false, -1.0f, 0, 2.0f);
    DrawDebugSphere(GetWorld(), GrappleTargetPoint, 20.0f, 8, FColor::Orange, false, -1.0f, 0, 2.0f);
#endif
}

void UGrapplingHookComponent::FireHook()
{
    if (!OwnerCharacter)
        return;

    // Release any existing grapple first
    if (bIsGrappling)
    {
        ReleaseHook();
        return;
    }

    FVector CamLoc;
    FVector CamFwd;
    GetCameraView(CamLoc, CamFwd);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        CamLoc,
        CamLoc + CamFwd * GrappleDistance,
        ECC_Visibility,
        Params
    );

    if (!bHit)
        return;

    // Accept both specifically-tagged surfaces AND any solid world geometry
    AActor* HitActor = HitResult.GetActor();
    bool bValidTarget = (HitActor && HitActor->ActorHasTag(GrappleTag))
                        || (HitResult.GetComponent() && !HitResult.GetComponent()->IsSimulatingPhysics());

    if (!bValidTarget)
        return;

    GrappleTargetPoint = HitResult.ImpactPoint;
    bIsGrappling = true;

    // Switch to flying mode so gravity doesn't fight the pull
    UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
    if (Movement)
    {
        Movement->SetMovementMode(MOVE_Flying);
    }
}

void UGrapplingHookComponent::ReleaseHook()
{
    if (!bIsGrappling || !OwnerCharacter)
        return;

    bIsGrappling = false;

    UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
    if (Movement)
    {
        // Restore falling movement to preserve inertia
        Movement->SetMovementMode(MOVE_Falling);
    }
}

void UGrapplingHookComponent::GetCameraView(FVector& OutLocation, FVector& OutForward) const
{
    if (OwnerCharacter)
    {
        UCameraComponent* Cam = OwnerCharacter->FindComponentByClass<UCameraComponent>();
        if (Cam)
        {
            OutLocation = Cam->GetComponentLocation();
            OutForward  = Cam->GetForwardVector();
            return;
        }
    }
    FRotator ViewRot;
    GetOwner()->GetActorEyesViewPoint(OutLocation, ViewRot);
    OutForward = ViewRot.Vector();
}
