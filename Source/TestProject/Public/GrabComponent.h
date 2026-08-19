#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "GrabComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TESTPROJECT_API UGrabComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGrabComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Attempt to grab the object the player is looking at */
    UFUNCTION(BlueprintCallable, Category = "Mechanics|Grab")
    void TryGrab();

    /** Release the currently held object. ThrowForce > 0 launches it forward */
    UFUNCTION(BlueprintCallable, Category = "Mechanics|Grab")
    void Release(float ThrowForce = 0.0f);

    /** Returns true if something is currently held */
    UFUNCTION(BlueprintPure, Category = "Mechanics|Grab")
    bool IsHoldingObject() const { return GrabbedComponent != nullptr; }

protected:
    virtual void BeginPlay() override;

private:
    /** Max reach for picking up objects */
    UPROPERTY(EditDefaultsOnly, Category = "Grab", meta=(ClampMin=50, ClampMax=1000))
    float GrabDistance = 300.0f;

    /** How far in front of the camera to hold the object */
    UPROPERTY(EditDefaultsOnly, Category = "Grab", meta=(ClampMin=50, ClampMax=500))
    float HoldDistance = 200.0f;

    /** Speed at which thrown objects are launched */
    UPROPERTY(EditDefaultsOnly, Category = "Grab", meta=(ClampMin=100, ClampMax=5000))
    float DefaultThrowForce = 1200.0f;

    UPROPERTY()
    UPhysicsHandleComponent* PhysicsHandle = nullptr;

    UPROPERTY()
    UPrimitiveComponent* GrabbedComponent = nullptr;

    /** Helper: returns camera location and forward vector */
    void GetCameraView(FVector& OutLocation, FVector& OutForward) const;
};

