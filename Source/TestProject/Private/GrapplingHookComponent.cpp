#include "GrapplingHookComponent.h"
#include "GrabComponent.h"
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

    // Draw preview marker when not grappling
    if (!bIsGrappling)
    {
        if (OwnerCharacter)
        {
            const UGrabComponent* GrabComp = OwnerCharacter->FindComponentByClass<UGrabComponent>();
            if (GrabComp && GrabComp->IsHoldingObject())
            {
                return;
            }
        }

        FHitResult PreviewHit;
        if (FindGrappleTarget(PreviewHit))
        {
            DrawDebugSphere(GetWorld(), PreviewHit.ImpactPoint, 20.0f, 8, FColor::Yellow, false, 0.0f, 0, 3.0f);
        }
        return;
    }

    if (!OwnerCharacter)
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

bool UGrapplingHookComponent::FindGrappleTarget(FHitResult& OutHit) const
{
    FVector CamLoc;
    FVector CamFwd;
    GetCameraView(CamLoc, CamFwd);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    FCollisionShape Sphere = FCollisionShape::MakeSphere(SweepRadius);

    bool bHit = GetWorld()->SweepSingleByChannel(
        OutHit,
        CamLoc,
        CamLoc + CamFwd * GrappleDistance,
        FQuat::Identity,
        ECC_Visibility,
        Sphere,
        Params
    );

    if (!bHit)
        return false;

    AActor* HitActor = OutHit.GetActor();
    return (HitActor && HitActor->ActorHasTag(GrappleTag));
}

void UGrapplingHookComponent::FireHook()
{
    if (!OwnerCharacter)
        return;

    if (bIsGrappling)
    {
        ReleaseHook();
        return;
    }

    // Do not allow grappling if the character is currently holding an object
    const UGrabComponent* GrabComp = OwnerCharacter->FindComponentByClass<UGrabComponent>();
    if (GrabComp && GrabComp->IsHoldingObject())
    {
        return;
    }

    FHitResult HitResult;
    if (!FindGrappleTarget(HitResult))
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
