#include "Interaction/TransformManager.h"

#include "MeshSceneSubsystem.h"
#include "ToolsSubsystem.h"
#include "BaseGizmos/CombinedTransformGizmo.h"
#include "BaseGizmos/TransformProxy.h"
#include "BaseGizmos/TransformGizmoUtil.h"

void USceneObjectTransformInteraction::Initialize(TUniqueFunction<bool()> GizmoEnabledCallbackIn) {
    GizmoEnabledCallback = MoveTemp(GizmoEnabledCallbackIn);

    SelectionChangedEventHandle = UMeshSceneSubsystem::Get()->OnSelectionModified.AddLambda(
        [this](UMeshSceneSubsystem* SceneSubsystem) { UpdateGizmoTargets(SceneSubsystem->GetSelection()); }
    );
}

void USceneObjectTransformInteraction::Shutdown() {
    if (SelectionChangedEventHandle.IsValid()) {
        if (UMeshSceneSubsystem::Get()) {
            UMeshSceneSubsystem::Get()->OnSelectionModified.Remove(SelectionChangedEventHandle);
        }
        SelectionChangedEventHandle = FDelegateHandle();
    }

    TArray<USceneObject*> EmptySelection;
    UpdateGizmoTargets(EmptySelection);
}


void USceneObjectTransformInteraction::SetEnableScaling(bool bEnable) {
    if (bEnable != bEnableScaling) {
        bEnableScaling = bEnable;
        ForceUpdateGizmoState();
    }
}

void USceneObjectTransformInteraction::SetEnableNonUniformScaling(bool bEnable) {
    if (bEnable != bEnableNonUniformScaling) {
        bEnableNonUniformScaling = bEnable;
        ForceUpdateGizmoState();
    }
}

void USceneObjectTransformInteraction::ForceUpdateGizmoState() {
    if (UMeshSceneSubsystem::Get()) {
        UpdateGizmoTargets(UMeshSceneSubsystem::Get()->GetSelection());
    }
}


void USceneObjectTransformInteraction::UpdateGizmoTargets(const TArray<USceneObject*>& Selection) {
    UInteractiveGizmoManager* GizmoManager = UToolsSubsystem::Get()->ToolsContext->GizmoManager;

    // destroy existing gizmos if we have any
    if (TransformGizmo != nullptr) {
        GizmoManager->DestroyAllGizmosByOwner(this);
        TransformGizmo = nullptr;
        TransformProxy = nullptr;
    }

    // if no selection, no gizmo
    if (Selection.Num() == 0 || GizmoEnabledCallback() == false) {
        return;
    }

    TransformProxy = NewObject<UTransformProxy>(this);
    for (USceneObject* SO : Selection) {
        // would be nice if this worked on Actors...
        TransformProxy->AddComponent(SO->GetMeshComponent());
    }

    ETransformGizmoSubElements GizmoElements = ETransformGizmoSubElements::FullTranslateRotateScale;
    if (bEnableScaling == false) {
        GizmoElements = ETransformGizmoSubElements::StandardTranslateRotate;
    } else if (bEnableNonUniformScaling == false || Selection.Num() > 1)  // cannot nonuniform-scale multiple objects
    {
        GizmoElements = ETransformGizmoSubElements::TranslateRotateUniformScale;
    }

    TransformGizmo = UE::TransformGizmoUtil::CreateCustomTransformGizmo(GizmoManager, GizmoElements, this);
    TransformGizmo->SetActiveTarget(TransformProxy);

    // optionally ignore coordinate system setting
    // TransformGizmo->bUseContextCoordinateSystem = false;
    // TransformGizmo->CurrentCoordinateSystem = EToolContextCoordinateSystem::Local;
}