#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrapplingHookComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TESTPROJECT_API UGrapplingHookComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGrapplingHookComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Fire the grappling hook towards the crosshair. Attaches if a GrappleSurface is hit. */
    UFUNCTION(BlueprintCallable, Category = "Mechanics|Grapple")
    void FireHook();

    /** Release the hook and restore normal movement */
    UFUNCTION(BlueprintCallable, Category = "Mechanics|Grapple")
    void ReleaseHook();

    /** Returns true while the hook is actively pulling the character */
    UFUNCTION(BlueprintPure, Category = "Mechanics|Grapple")
    bool IsGrappling() const { return bIsGrappling; }

    /** Sweep for a valid grapple target and return hit info */
    bool FindGrappleTarget(FHitResult& OutHit) const;

protected:
    virtual void BeginPlay() override;

private:
    /** Sphere radius for the grapple aim sweep in cm */
    UPROPERTY(EditDefaultsOnly, Category = "Grapple", meta=(ClampMin=5, ClampMax=100))
    float SweepRadius = 15.0f;

    /** Maximum hook reach in cm */
    UPROPERTY(EditDefaultsOnly, Category = "Grapple", meta=(ClampMin=500, ClampMax=5000))
    float GrappleDistance = 2500.0f;

    /** Base pull speed towards the grapple point */
    UPROPERTY(EditDefaultsOnly, Category = "Grapple", meta=(ClampMin=100, ClampMax=5000))
    float PullSpeed = 2000.0f;

    /** How close to the grapple point before auto-releasing */
    UPROPERTY(EditDefaultsOnly, Category = "Grapple", meta=(ClampMin=10, ClampMax=300))
    float ArrivalRadius = 80.0f;

    /** Tag that marks surfaces/points the hook can attach to */
    UPROPERTY(EditDefaultsOnly, Category = "Grapple")
    FName GrappleTag = "GrappleSurface";

    bool bIsGrappling = false;
    FVector GrappleTargetPoint = FVector::ZeroVector;

    /** Helper: returns camera location and forward vector */
    void GetCameraView(FVector& OutLocation, FVector& OutForward) const;

    class ACharacter* OwnerCharacter = nullptr;
};
