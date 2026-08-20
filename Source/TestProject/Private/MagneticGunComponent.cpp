#include "MagneticGunComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

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

    // Chaos Physics aggressively sleeps bodies — must wake every tick or forces are ignored
    TargetComponent->WakeRigidBody();

    FVector CamLoc;
    FVector CamFwd;
    GetCameraView(CamLoc, CamFwd);

    FVector PullPoint = CamLoc + CamFwd * 150.0f; // point in front of player
    FVector ObjectLoc = TargetComponent->GetComponentLocation();
    FVector Direction = (PullPoint - ObjectLoc);
    float   Distance  = Direction.Size();

    if (Distance < 30.0f)
    {
        // Close enough — stop the object gently
        FVector Vel = TargetComponent->GetPhysicsLinearVelocity();
        TargetComponent->SetPhysicsLinearVelocity(Vel * FMath::Max(0.0f, 1.0f - DeltaTime * 10.0f));
        return;
    }

    Direction /= Distance; // normalize

    // Target speed: strong pull from far, slows as it approaches
    float TargetSpeed = FMath::Clamp(Distance * 1.5f, 80.0f, AttractForceBase);

    FVector TargetVelocity  = Direction * TargetSpeed;
    FVector CurrentVelocity = TargetComponent->GetPhysicsLinearVelocity();

    // Smoothly steer the velocity toward the pull direction
    FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, 5.0f);
    TargetComponent->SetPhysicsLinearVelocity(NewVelocity);
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

    Target->WakeRigidBody();
    Target->AddImpulse(Direction * RepelImpulse, NAME_None, /*bVelChange=*/true);
}

void UMagneticGunComponent::ToggleMode()
{
    StopAttract();
    CurrentMode = (CurrentMode == EMagneticMode::Attract) ? EMagneticMode::Repel : EMagneticMode::Attract;
}

UPrimitiveComponent* UMagneticGunComponent::FindMagneticTarget() const
{
    FVector CamLoc;
    FVector CamFwd;
    GetCameraView(CamLoc, CamFwd);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    // Try multiple channels so we catch objects regardless of their collision preset
    const ECollisionChannel Channels[] = { ECC_PhysicsBody, ECC_WorldDynamic, ECC_WorldStatic, ECC_Visibility };

    for (ECollisionChannel Channel : Channels)
    {
        TArray<FHitResult> Hits;

        GetWorld()->SweepMultiByChannel(
            Hits,
            CamLoc,
            CamLoc + CamFwd * MagnetRange,
            FQuat::Identity,
            Channel,
            FCollisionShape::MakeSphere(TraceRadius),
            Params
        );

        for (const FHitResult& Hit : Hits)
        {
            if (!Hit.GetActor())
                continue;

            bool bHasTag = Hit.GetActor()->ActorHasTag(MagneticTag) || Hit.GetComponent()->ComponentHasTag(MagneticTag);
            if (!bHasTag || !Hit.GetComponent()->IsSimulatingPhysics())
                continue;

            return Hit.GetComponent();
        }
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
