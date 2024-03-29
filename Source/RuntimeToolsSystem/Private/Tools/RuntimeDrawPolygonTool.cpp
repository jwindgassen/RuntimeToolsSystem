#include "Tools/RuntimeDrawPolygonTool.h"
#include "MeshSceneSubsystem.h"

#define LOCTEXT_NAMESPACE "URuntimeDrawPolygonTool"

UInteractiveTool* URuntimeDrawPolygonToolBuilder::BuildTool(const FToolBuilderState& SceneState) const {
    URuntimeDrawPolygonTool* NewTool = NewObject<URuntimeDrawPolygonTool>(SceneState.ToolManager);
    NewTool->SetWorld(SceneState.World);
    return NewTool;
}



void URuntimeDrawPolygonTool::Setup() {
    UDrawPolygonTool::Setup();

    // initialize to drawing material
    this->MaterialProperties->Material = UMeshSceneSubsystem::Get()->StandardMaterial;

    // mirror properties we want to expose at runtime
    RuntimeProperties = NewObject<URuntimeDrawPolygonToolProperties>(this);

    RuntimeProperties->SelectedPolygonType = static_cast<int>(PolygonProperties->PolygonDrawMode);
    RuntimeProperties->WatchProperty(RuntimeProperties->SelectedPolygonType, [this](int NewType) {
        PolygonProperties->PolygonDrawMode = static_cast<EDrawPolygonDrawMode>(NewType);
    });

    AddToolPropertySource(RuntimeProperties);
}


#undef LOCTEXT_NAMESPACE