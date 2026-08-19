#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MagneticGunComponent.generated.h"

UENUM(BlueprintType)
enum class EMagneticMode : uint8
{
    Attract UMETA(DisplayName = "Attract"),
    Repel   UMETA(DisplayName = "Repel")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TESTPROJECT_API UMagneticGunComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMagneticGunComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Begin attracting the targeted magnetic object (call on button pressed) */
    UFUNCTION(BlueprintCallable, Category = "Mechanics|Magnet")
    void StartAttract();

    /** Stop attracting (call on button released) */
    UFUNCTION(BlueprintCallable, Category = "Mechanics|Magnet")
    void StopAttract();

    /** Fire a single repulsion impulse at the targeted object */
    UFUNCTION(BlueprintCallable, Category = "Mechanics|Magnet")
    void Repel();

    /** Switches between Attract and Repel modes */
    UFUNCTION(BlueprintCallable, Category = "Mechanics|Magnet")
    void ToggleMode();

    /** Returns current mode */
    UFUNCTION(BlueprintPure, Category = "Mechanics|Magnet")
    EMagneticMode GetCurrentMode() const { return CurrentMode; }

    // Legacy single-call versions kept for backward compatibility
    UFUNCTION(BlueprintCallable, Category = "Mechanics|Magnet")
    void Attract() { StartAttract(); }

protected:
    virtual void BeginPlay() override;

private:
    /** Max distance for magnet ray (cm) */
    UPROPERTY(EditDefaultsOnly, Category = "Magnet", meta=(ClampMin=100, ClampMax=5000))
    float MagnetRange = 1500.0f;

    /** Cone half-angle for sphere-trace assist */
    UPROPERTY(EditDefaultsOnly, Category = "Magnet", meta=(ClampMin=1, ClampMax=50))
    float TraceRadius = 30.0f;

    /** Force applied per tick while attracting (scales with 1/distance) */
    UPROPERTY(EditDefaultsOnly, Category = "Magnet", meta=(ClampMin=100, ClampMax=100000))
    float AttractForceBase = 15000.0f;

    /** Single-shot repulsion impulse strength */
    UPROPERTY(EditDefaultsOnly, Category = "Magnet", meta=(ClampMin=100, ClampMax=100000))
    float RepelImpulse = 8000.0f;

    /** Tag to identify magnetic / metal objects */
    UPROPERTY(EditDefaultsOnly, Category = "Magnet")
    FName MagneticTag = "Magnetic";

    EMagneticMode CurrentMode = EMagneticMode::Attract;
    bool bIsAttracting = false;

    UPROPERTY()
    UPrimitiveComponent* TargetComponent = nullptr;

    /** Scan for the best magnetic target in front of the camera */
    UPrimitiveComponent* FindMagneticTarget() const;

    /** Returns camera location and forward vector */
    void GetCameraView(FVector& OutLocation, FVector& OutForward) const;

    class ACharacter* OwnerCharacter = nullptr;
};
