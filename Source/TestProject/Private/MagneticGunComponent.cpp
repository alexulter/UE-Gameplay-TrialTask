#include "MagneticGunComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UMagneticGunComponent::UMagneticGunComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UMagneticGunComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UMagneticGunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsAttracting || !TargetComponent)
        return;

    FVector CamLoc;
    FVector CamFwd;
    GetCameraView(CamLoc, CamFwd);

    FVector ObjectLoc = TargetComponent->GetComponentLocation();
    FVector Direction = (CamLoc + CamFwd * 150.0f) - ObjectLoc; // pull towards a point in front of player
    float Distance = FMath::Max(Direction.Size(), 50.0f);        // avoid division by zero
    Direction.Normalize();

    // Force diminishes with distance (F = Base / Distance), scaled by DeltaTime
    float Force = AttractForceBase / Distance * 100.0f;
    TargetComponent->AddForce(Direction * Force, NAME_None, /*bAccelChange=*/false);

#if WITH_EDITOR
    DrawDebugLine(GetWorld(), CamLoc, ObjectLoc, FColor::Cyan, false, -1.0f, 0, 1.5f);
#endif
}

void UMagneticGunComponent::StartAttract()
{
    if (bIsAttracting)
        return;

    TargetComponent = FindMagneticTarget();
    if (!TargetComponent)
        return;

    bIsAttracting = true;
}

void UMagneticGunComponent::StopAttract()
{
    bIsAttracting = false;
    TargetComponent = nullptr;
}

void UMagneticGunComponent::Repel()
{
    UPrimitiveComponent* Target = FindMagneticTarget();
    if (!Target)
        return;

    FVector CamLoc;
    FVector CamFwd;
    GetCameraView(CamLoc, CamFwd);

    FVector ObjectLoc = Target->GetComponentLocation();
    FVector Direction = (ObjectLoc - CamLoc).GetSafeNormal();

    Target->AddImpulse(Direction * RepelImpulse, NAME_None, /*bVelChange=*/true);

#if WITH_EDITOR
    DrawDebugLine(GetWorld(), CamLoc, ObjectLoc, FColor::Red, false, 0.3f, 0, 3.0f);
#endif
}

void UMagneticGunComponent::ToggleMode()
{
    StopAttract(); // stop any active beam first
    CurrentMode = (CurrentMode == EMagneticMode::Attract) ? EMagneticMode::Repel : EMagneticMode::Attract;
}

UPrimitiveComponent* UMagneticGunComponent::FindMagneticTarget() const
{
    FVector CamLoc;
    FVector CamFwd;
    GetCameraView(CamLoc, CamFwd);

    TArray<FHitResult> Hits;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    // Sphere sweep along camera ray to give a generous targeting cone
    GetWorld()->SweepMultiByChannel(
        Hits,
        CamLoc,
        CamLoc + CamFwd * MagnetRange,
        FQuat::Identity,
        ECC_PhysicsBody,
        FCollisionShape::MakeSphere(TraceRadius),
        Params
    );

    for (const FHitResult& Hit : Hits)
    {
        if (!Hit.GetActor())
            continue;
        if (!Hit.GetActor()->ActorHasTag(MagneticTag))
            continue;
        if (!Hit.GetComponent()->IsSimulatingPhysics())
            continue;
        return Hit.GetComponent();
    }

    return nullptr;
}

void UMagneticGunComponent::GetCameraView(FVector& OutLocation, FVector& OutForward) const
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
