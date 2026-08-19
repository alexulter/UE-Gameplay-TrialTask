#include "GrabComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UGrabComponent::UGrabComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UGrabComponent::BeginPlay()
{
    Super::BeginPlay();
    // PhysicsHandleComponent must be added to the owning actor in the editor or constructor
    PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
    if (!PhysicsHandle)
    {
        UE_LOG(LogTemp, Warning, TEXT("GrabComponent: No PhysicsHandleComponent found on %s"), *GetOwner()->GetName());
    }
}

void UGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!PhysicsHandle || !GrabbedComponent)
        return;

    // Update the target location to always float in front of the camera
    FVector CamLoc;
    FVector CamFwd;
    GetCameraView(CamLoc, CamFwd);

    FVector TargetLocation = CamLoc + CamFwd * HoldDistance;
    PhysicsHandle->SetTargetLocationAndRotation(TargetLocation, GetOwner()->GetActorRotation());
}

void UGrabComponent::TryGrab()
{
    if (!PhysicsHandle)
        return;

    // If already holding something, drop it
    if (GrabbedComponent)
    {
        Release(0.0f);
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
        CamLoc + CamFwd * GrabDistance,
        ECC_PhysicsBody,
        Params
    );

    if (!bHit || !HitResult.GetComponent())
        return;

    UPrimitiveComponent* HitComp = HitResult.GetComponent();

    // Only grab objects tagged "Grabbable" and not "Heavy"
    AActor* HitActor = HitResult.GetActor();
    if (!HitActor || !HitActor->ActorHasTag("Grabbable") || HitActor->ActorHasTag("Heavy"))
        return;

    if (!HitComp->IsSimulatingPhysics())
        return;

    GrabbedComponent = HitComp;
    PhysicsHandle->GrabComponentAtLocationWithRotation(
        GrabbedComponent,
        NAME_None,
        GrabbedComponent->GetComponentLocation(),
        GrabbedComponent->GetComponentRotation()
    );
}

void UGrabComponent::Release(float ThrowForce)
{
    if (!PhysicsHandle || !GrabbedComponent)
        return;

    PhysicsHandle->ReleaseComponent();

    if (ThrowForce > 0.0f)
    {
        FVector CamLoc;
        FVector CamFwd;
        GetCameraView(CamLoc, CamFwd);
        GrabbedComponent->AddImpulse(CamFwd * ThrowForce, NAME_None, true);
    }

    GrabbedComponent = nullptr;
}

void UGrabComponent::GetCameraView(FVector& OutLocation, FVector& OutForward) const
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (OwnerChar)
    {
        UCameraComponent* Cam = OwnerChar->FindComponentByClass<UCameraComponent>();
        if (Cam)
        {
            OutLocation = Cam->GetComponentLocation();
            OutForward  = Cam->GetForwardVector();
            return;
        }
    }
    // Fallback: use actor's eye viewpoint
    FRotator ViewRot;
    GetOwner()->GetActorEyesViewPoint(OutLocation, ViewRot);
    OutForward = ViewRot.Vector();
}
