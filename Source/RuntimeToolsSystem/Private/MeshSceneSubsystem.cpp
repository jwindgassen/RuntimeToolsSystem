#include "MeshSceneSubsystem.h"
#include "MaterialDomain.h"
#include "Interaction/SceneObject.h"
#include "Materials/Material.h"


#define LOCTEXT_NAMESPACE "UMeshSceneSubsystem"


UMeshSceneSubsystem* UMeshSceneSubsystem::InstanceSingleton = nullptr;


void UMeshSceneSubsystem::InitializeSingleton(UMeshSceneSubsystem* Subsystem) {
    check(InstanceSingleton == nullptr);
    InstanceSingleton = Subsystem;

    // todo: expose these somehow?

    UMaterial* DefaultObjectMaterial =
        LoadObject<UMaterial>(nullptr, TEXT("/Game/RuntimeToolsFrameworkMaterials/DefaultObjectMaterial"));
    InstanceSingleton->StandardMaterial =
        (DefaultObjectMaterial) ? DefaultObjectMaterial : UMaterial::GetDefaultMaterial(MD_Surface);

    UMaterial* SelectionMaterial =
        LoadObject<UMaterial>(nullptr, TEXT("/Game/RuntimeToolsFrameworkMaterials/SelectedMaterial"));
    InstanceSingleton->SelectedMaterial =
        (SelectionMaterial) ? SelectionMaterial : UMaterial::GetDefaultMaterial(MD_Surface);

    UMaterial* WireframeMaterial =
        LoadObject<UMaterial>(nullptr, TEXT("/Game/RuntimeToolsFrameworkMaterials/WireframeMaterial"));
    WireframeMaterial = (WireframeMaterial) ? WireframeMaterial : UMaterial::GetDefaultMaterial(MD_Surface);
    InstanceSingleton->WireframeMaterial = WireframeMaterial;
    GEngine->WireframeMaterial = WireframeMaterial;
}


UMeshSceneSubsystem* UMeshSceneSubsystem::Get() {
    return InstanceSingleton;
}


void UMeshSceneSubsystem::Deinitialize() {
    InstanceSingleton = nullptr;
}


void UMeshSceneSubsystem::SetCurrentTransactionsAPI(IToolsContextTransactionsAPI* TransactionsAPIIn) {
    TransactionsAPI = TransactionsAPIIn;
}


USceneObject* UMeshSceneSubsystem::CreateNewSceneObject() {
    USceneObject* SceneObject = NewObject<USceneObject>(this);
    AddSceneObjectInternal(SceneObject, false);

    if (TransactionsAPI) {
        TUniquePtr<FAddRemoveSceneObjectChange> AddChange = MakeUnique<FAddRemoveSceneObjectChange>();
        AddChange->SceneObject = SceneObject;
        AddChange->bAdded = true;
        // use SceneObject as target so that transaction will keep it from being GC'd
        TransactionsAPI->AppendChange(SceneObject, MoveTemp(AddChange), LOCTEXT("AddObjectChange", "Add SceneObject"));
    }

    SceneObject->SetAllMaterials(StandardMaterial);

    return SceneObject;
}


USceneObject* UMeshSceneSubsystem::FindSceneObjectByActor(AActor* Actor) {
    for (USceneObject* SceneObject : SceneObjects) {
        if (SceneObject->GetActor() == Actor) {
            return SceneObject;
        }
    }
    return nullptr;
}


bool UMeshSceneSubsystem::DeleteSceneObject(USceneObject* SceneObject) {
    if (SceneObjects.Contains(SceneObject)) {
        if (SelectedSceneObjects.Contains(SceneObject)) {
            BeginSelectionChange();
            SelectedSceneObjects.Remove(SceneObject);
            EndSelectionChange();
            OnSelectionModified.Broadcast(this);
        }

        RemoveSceneObjectInternal(SceneObject, true);

        if (TransactionsAPI) {
            TUniquePtr<FAddRemoveSceneObjectChange> RemoveChange = MakeUnique<FAddRemoveSceneObjectChange>();
            RemoveChange->SceneObject = SceneObject;
            RemoveChange->bAdded = false;
            // use SceneObject as target so that transaction will keep it from being GC'd
            TransactionsAPI->AppendChange(
                SceneObject, MoveTemp(RemoveChange), LOCTEXT("RemoveObjectChange", "Delete SceneObject")
            );
        }

        return true;
    }
    UE_LOG(
        LogTemp, Warning,
        TEXT("[UMeshSceneSubsystem::DeleteSceneObject] Tried to delete non-existant SceneObject")
    );
    return false;
}


bool UMeshSceneSubsystem::DeleteSelectedSceneObjects() {
    return DeleteSelectedSceneObjects(nullptr);
}

bool UMeshSceneSubsystem::DeleteSelectedSceneObjects(AActor* SkipActor) {
    if (SelectedSceneObjects.Num() == 0) {
        return false;
    }

    if (TransactionsAPI) {
        TransactionsAPI->BeginUndoTransaction(LOCTEXT("DeleteSelectedObjectsChange", "Delete Objects"));
    }

    TArray<USceneObject*> DeleteObjects = SelectedSceneObjects;

    BeginSelectionChange();
    SelectedSceneObjects.Reset();
    EndSelectionChange();

    for (USceneObject* SceneObject : DeleteObjects) {
        if (SceneObject->GetActor() == SkipActor) {
            continue;
        }

        RemoveSceneObjectInternal(SceneObject, true);

        if (TransactionsAPI) {
            TUniquePtr<FAddRemoveSceneObjectChange> RemoveChange = MakeUnique<FAddRemoveSceneObjectChange>();
            RemoveChange->SceneObject = SceneObject;
            RemoveChange->bAdded = false;
            // use SceneObject as target so that transaction will keep it from being GC'd
            TransactionsAPI->AppendChange(
                SceneObject, MoveTemp(RemoveChange), LOCTEXT("RemoveObjectChange", "Delete SceneObject")
            );
        }
    }

    if (TransactionsAPI) {
        TransactionsAPI->EndUndoTransaction();
    }

    OnSelectionModified.Broadcast(this);
    return true;
}



void UMeshSceneSubsystem::ClearSelection() {
    if (SelectedSceneObjects.Num() > 0) {
        BeginSelectionChange();

        for (USceneObject* SO : SelectedSceneObjects) {
            SO->ClearHighlightMaterial();
        }
        SelectedSceneObjects.Reset();

        EndSelectionChange();
        OnSelectionModified.Broadcast(this);
    }
}


void UMeshSceneSubsystem::SetSelected(
    USceneObject* SceneObject, bool bDeselect, bool bDeselectOthers
) {
    if (bDeselect) {
        if (SelectedSceneObjects.Contains(SceneObject)) {
            BeginSelectionChange();
            SelectedSceneObjects.Remove(SceneObject);
            SceneObject->ClearHighlightMaterial();
            EndSelectionChange();
            OnSelectionModified.Broadcast(this);
        }
    } else {
        BeginSelectionChange();

        bool bIsSelected = SelectedSceneObjects.Contains(SceneObject);
        if (bDeselectOthers) {
            for (USceneObject* SO : SelectedSceneObjects) {
                if (SO != SceneObject) {
                    SO->ClearHighlightMaterial();
                }
            }
            SelectedSceneObjects.Reset();
        }
        if (bIsSelected == false) {
            SceneObject->SetToHighlightMaterial(this->SelectedMaterial);
        }
        SelectedSceneObjects.Add(SceneObject);

        EndSelectionChange();
        OnSelectionModified.Broadcast(this);
    }
}


void UMeshSceneSubsystem::ToggleSelected(USceneObject* SceneObject) {
    BeginSelectionChange();

    if (SelectedSceneObjects.Contains(SceneObject)) {
        SelectedSceneObjects.Remove(SceneObject);
        SceneObject->ClearHighlightMaterial();
    } else {
        SelectedSceneObjects.Add(SceneObject);
        SceneObject->SetToHighlightMaterial(this->SelectedMaterial);
    }

    EndSelectionChange();
    OnSelectionModified.Broadcast(this);
}


void UMeshSceneSubsystem::SetSelection(const TArray<USceneObject*>& NewSceneObjects) {
    BeginSelectionChange();
    SetSelectionInternal(NewSceneObjects);
    EndSelectionChange();
}
void UMeshSceneSubsystem::SetSelectionInternal(const TArray<USceneObject*>& NewSceneObjects) {
    if (SelectedSceneObjects.Num() > 0) {
        for (USceneObject* SO : SelectedSceneObjects) {
            SO->ClearHighlightMaterial();
        }
        SelectedSceneObjects.Reset();
    }

    for (USceneObject* SO : NewSceneObjects) {
        if (SceneObjects.Contains(SO)) {
            if (SelectedSceneObjects.Contains(SO) == false) {
                SelectedSceneObjects.Add(SO);
                SO->SetToHighlightMaterial(this->SelectedMaterial);
            }
        } else {
            UE_LOG(
                LogTemp, Warning,
                TEXT("[UMeshSceneSubsystem::SetSelectionInternal] Tried to select non-existant SceneObject")
            );
        }
    }

    OnSelectionModified.Broadcast(this);
}



USceneObject* UMeshSceneSubsystem::FindNearestHitObject(
    FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle,
    FVector& TriBaryCoords, float MaxDistance
) {
    USceneObject* FoundHit = nullptr;
    float MinHitDistance = TNumericLimits<float>::Max();

    for (USceneObject* SO : SceneObjects) {
        FVector HitPoint, BaryCoords;
        float HitDist;
        int32 NearestTri;
        if (SO->IntersectRay(RayOrigin, RayDirection, HitPoint, HitDist, NearestTri, BaryCoords, MaxDistance)) {
            if (HitDist < MinHitDistance) {
                MinHitDistance = HitDist;
                WorldHitPoint = HitPoint;
                HitDistance = HitDist;
                NearestTriangle = NearestTri;
                TriBaryCoords = BaryCoords;
                FoundHit = SO;
            }
        }
    }
    return FoundHit;
}



void UMeshSceneSubsystem::BeginSelectionChange() {
    check(!ActiveSelectionChange);

    ActiveSelectionChange = MakeUnique<FMeshSceneSelectionChange>();
    ActiveSelectionChange->OldSelection = SelectedSceneObjects;
}

void UMeshSceneSubsystem::EndSelectionChange() {
    check(ActiveSelectionChange);
    if (SelectedSceneObjects != ActiveSelectionChange->OldSelection) {
        ActiveSelectionChange->NewSelection = SelectedSceneObjects;

        if (TransactionsAPI) {
            TransactionsAPI->AppendChange(
                this, MoveTemp(ActiveSelectionChange), LOCTEXT("SelectionChange", "Selection Change")
            );
        }
    }

    ActiveSelectionChange = nullptr;
}

void FMeshSceneSelectionChange::Apply(UObject* Object) {
    if (UMeshSceneSubsystem* Subsystem = Cast<UMeshSceneSubsystem>(Object)) {
        Subsystem->SetSelectionInternal(NewSelection);
    }
}
void FMeshSceneSelectionChange::Revert(UObject* Object) {
    if (UMeshSceneSubsystem* Subsystem = Cast<UMeshSceneSubsystem>(Object)) {
        Subsystem->SetSelectionInternal(OldSelection);
    }
}



void UMeshSceneSubsystem::AddSceneObjectInternal(USceneObject* Object, bool bIsUndoRedo) {
    SceneObjects.Add(Object);

    if (bIsUndoRedo) {
        Object->GetActor()->RegisterAllComponents();
    }
}

void UMeshSceneSubsystem::RemoveSceneObjectInternal(USceneObject* Object, bool bIsUndoRedo) {
    check(SceneObjects.Contains(Object));
    SceneObjects.Remove(Object);

    Object->GetActor()->UnregisterAllComponents(true);
}



void FAddRemoveSceneObjectChange::Apply(UObject* Object) {
    if (bAdded) {
        UMeshSceneSubsystem::Get()->AddSceneObjectInternal(SceneObject, true);
    } else {
        UMeshSceneSubsystem::Get()->RemoveSceneObjectInternal(SceneObject, true);
    }
}

void FAddRemoveSceneObjectChange::Revert(UObject* Object) {
    if (bAdded) {
        UMeshSceneSubsystem::Get()->RemoveSceneObjectInternal(SceneObject, true);
    } else {
        UMeshSceneSubsystem::Get()->AddSceneObjectInternal(SceneObject, true);
    }
}


#undef LOCTEXT_NAMESPACE