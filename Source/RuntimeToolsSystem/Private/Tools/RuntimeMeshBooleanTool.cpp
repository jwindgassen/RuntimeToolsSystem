#include "Tools/RuntimeMeshBooleanTool.h"
#include "ToolsSubsystem.h"
#include "MeshSceneSubsystem.h"
#include "ModelingToolTargetUtil.h"

#define LOCTEXT_NAMESPACE "URuntimeMeshBooleanTool"

using namespace UE::Geometry;

UMultiSelectionMeshEditingTool* URuntimeMeshBooleanToolBuilder::CreateNewTool(const FToolBuilderState& SceneState
) const {
    return NewObject<URuntimeMeshBooleanTool>(SceneState.ToolManager);
}

void URuntimeMeshBooleanTool::Setup() {
    UCSGMeshesTool::Setup();

    this->CSGProperties->bTryFixHoles = true;
    // write to first input asset
    this->HandleSourcesProperties->OutputWriteTo = EBaseCreateFromSelectedTargetType::FirstInputObject;
    // set to keep sources because we will handle deleting SceneObjects ourselves. We cannot
    // allow the Tool to do it because it will just call Actor.Destroy(), but we actually want
    // to keep the Actor around for undo/redo
    this->HandleSourcesProperties->HandleInputs = EHandleSourcesMethod::KeepSources;

    // mirror properties we want to expose at runtime
    RuntimeProperties = NewObject<URuntimeMeshBooleanToolProperties>(this);

    RuntimeProperties->OperationType = static_cast<int>(CSGProperties->Operation);
    RuntimeProperties->WatchProperty(RuntimeProperties->OperationType, [this](int NewType) {
        CSGProperties->Operation = static_cast<ECSGOperation>(NewType);
        Preview->InvalidateResult();
    });

    AddToolPropertySource(RuntimeProperties);
}


void URuntimeMeshBooleanTool::Shutdown(EToolShutdownType ShutdownType) {
    if (ShutdownType == EToolShutdownType::Accept) {
        GetToolManager()->BeginUndoTransaction(GetActionName());

        // base UCSGMeshesTool will delete the Actor but we need to also delete the SO...
        AActor* KeepActor = UE::ToolTarget::GetTargetActor(Targets[0]);
        USceneObject* KeepSO = UMeshSceneSubsystem::Get()->FindSceneObjectByActor(KeepActor);
        UMeshSceneSubsystem::Get()->SetSelected(KeepSO, true, false);
        UMeshSceneSubsystem::Get()->DeleteSelectedSceneObjects(KeepActor);
    }

    UCSGMeshesTool::Shutdown(ShutdownType);

    if (ShutdownType == EToolShutdownType::Accept) {
        GetToolManager()->EndUndoTransaction();
    }
}


#undef LOCTEXT_NAMESPACE