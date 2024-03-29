#include "Tools/RuntimeDynamicMeshSculptTool.h"
#include "ToolsSubsystem.h"

#include "ToolBuilderUtil.h"

#define LOCTEXT_NAMESPACE "URuntimeDynamicMeshSculptTool"

UMeshSurfacePointTool* URuntimeDynamicMeshSculptToolBuilder::CreateNewTool(const FToolBuilderState& SceneState) const {
    URuntimeDynamicMeshSculptTool* SculptTool = NewObject<URuntimeDynamicMeshSculptTool>(SceneState.ToolManager);
    SculptTool->SetEnableRemeshing(this->bEnableRemeshing);
    SculptTool->SetWorld(SceneState.World);
    return SculptTool;
}



static ERuntimeDynamicMeshSculptBrushType Convert(EDynamicMeshSculptBrushType BrushType) {
    switch (BrushType) {
        case EDynamicMeshSculptBrushType::Move: return ERuntimeDynamicMeshSculptBrushType::Move;
        case EDynamicMeshSculptBrushType::Smooth: return ERuntimeDynamicMeshSculptBrushType::Smooth;
        case EDynamicMeshSculptBrushType::Inflate: return ERuntimeDynamicMeshSculptBrushType::Inflate;
        case EDynamicMeshSculptBrushType::PlaneViewAligned: return ERuntimeDynamicMeshSculptBrushType::Flatten;
        default: return ERuntimeDynamicMeshSculptBrushType::Sculpt;
    }
}
static EDynamicMeshSculptBrushType Convert(ERuntimeDynamicMeshSculptBrushType BrushType) {
    switch (BrushType) {
        case ERuntimeDynamicMeshSculptBrushType::Move: return EDynamicMeshSculptBrushType::Move;
        case ERuntimeDynamicMeshSculptBrushType::Smooth: return EDynamicMeshSculptBrushType::Smooth;
        case ERuntimeDynamicMeshSculptBrushType::Inflate: return EDynamicMeshSculptBrushType::Inflate;
        case ERuntimeDynamicMeshSculptBrushType::Flatten: return EDynamicMeshSculptBrushType::PlaneViewAligned;
        default: return EDynamicMeshSculptBrushType::Offset;
    }
}


void URuntimeDynamicMeshSculptTool::Setup() {
    UDynamicMeshSculptTool::Setup();

    // mirror properties we want to expose at runtime
    RuntimeProperties = NewObject<URuntimeDynamicMeshSculptToolProperties>(this);

    RuntimeProperties->BrushSize = BrushProperties->BrushSize.AdaptiveSize;
    RuntimeProperties->WatchProperty(RuntimeProperties->BrushSize, [this](float NewValue) {
        BrushProperties->BrushSize.AdaptiveSize = NewValue;
        OnPropertyModified(
            nullptr,
            nullptr
        );  // hack to get CalculateBrushRadius() to be called, because it is private (why?)
    });

    RuntimeProperties->BrushStrength = SculptProperties->PrimaryBrushSpeed;
    RuntimeProperties->WatchProperty(RuntimeProperties->BrushStrength, [this](float NewValue) {
        SculptProperties->PrimaryBrushSpeed = NewValue;
    });

    RuntimeProperties->BrushFalloff = BrushProperties->BrushFalloffAmount;
    RuntimeProperties->WatchProperty(RuntimeProperties->BrushFalloff, [this](float NewValue) {
        BrushProperties->BrushFalloffAmount = NewValue;
    });

    RuntimeProperties->SelectedBrushType = static_cast<int>(Convert(SculptProperties->PrimaryBrushType));
    RuntimeProperties->WatchProperty(RuntimeProperties->SelectedBrushType, [this](int NewType) {
        SculptProperties->PrimaryBrushType = Convert(static_cast<ERuntimeDynamicMeshSculptBrushType>(NewType));
    });

    AddToolPropertySource(RuntimeProperties);
}


#undef LOCTEXT_NAMESPACE