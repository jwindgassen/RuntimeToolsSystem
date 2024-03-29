#pragma once

#include "TransformManager.generated.h"

class UTransformProxy;
class UCombinedTransformGizmo;
class USceneObject;

/**
 * USceneObjectTransformInteraction manages a 3D Translate/Rotate/Scale (TRS) Gizmo for the
 * current USceneObject selection set (stored in URuntimeMeshSceneSubsystem).
 *
 * Gizmo local/global frame is not controlled here, the Gizmo looks this information up itself
 * based on the EToolContextCoordinateSystem provided by the IToolsContextQueriesAPI implementation
 * in UToolsSubsystem. You can configure the Gizmo to ignore this, in UpdateGizmoTargets()
 *
 * Behavior of the TRS Gizmo (ie pivot position, etc) is controlled by a standard UTransformProxy.
 * See UTransformMeshesTool for sample code for doing things like modifying pivot dynamically/etc.
 */
UCLASS()
class USceneObjectTransformInteraction : public UObject {
    GENERATED_BODY()
public:
    /**
     * Set up the transform interaction.
     * @param GizmoEnabledCallbackIn callback that determines if Gizmo should be created and visible. For example during
     * a Tool we generally want to hide the TRS Gizmo.
     */
    void Initialize(TUniqueFunction<bool()> GizmoEnabledCallbackIn);

    void Shutdown();

    UFUNCTION(BlueprintCallable)
    void SetEnableScaling(bool bEnable);

    UFUNCTION(BlueprintCallable)
    void SetEnableNonUniformScaling(bool bEnable);

    // Recreate Gizmo. Call when external state changes, like set of selected objects
    UFUNCTION(BlueprintCallable)
    void ForceUpdateGizmoState();

protected:
    FDelegateHandle SelectionChangedEventHandle;

    UPROPERTY()
    UTransformProxy* TransformProxy;

    UPROPERTY()
    UCombinedTransformGizmo* TransformGizmo;

    void UpdateGizmoTargets(const TArray<USceneObject*>& Selection);

    bool bEnableScaling = true;
    bool bEnableNonUniformScaling = true;

    TUniqueFunction<bool()> GizmoEnabledCallback = [&]() { return true; };
};