#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InteractiveToolsContext.h"
#include "MeshSceneSubsystem.generated.h"

class FMeshSceneSelectionChange;
class FAddRemoveSceneObjectChange;
class USceneObject;

/**
 * UMeshSceneSubsystem manages a "Scene" of "SceneObjects", currently only USceneObject (SO).
 *
 * Use CreateNewSceneObject() to create a new SO, and the various Delete functions to remove them.
 * These changes will be undo-able, ie they will send Change events to the USceneHistoryManager instance.
 *
 * An active Selection Set is tracked, and there are API functions for modifying this Selection set, also undo-able.
 *
 * Cast rays into the scene using FindNearestHitObject()
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API UMeshSceneSubsystem : public UGameInstanceSubsystem {
    GENERATED_BODY()

public:
    static void InitializeSingleton(UMeshSceneSubsystem* Subsystem);
    static UMeshSceneSubsystem* Get();

protected:
    static UMeshSceneSubsystem* InstanceSingleton;

public:
    virtual void Deinitialize() override;


    virtual void SetCurrentTransactionsAPI(IToolsContextTransactionsAPI* TransactionsAPI);


public:
    UPROPERTY()
    UMaterialInterface* StandardMaterial;

    UPROPERTY()
    UMaterialInterface* SelectedMaterial;

    UPROPERTY()
    UMaterialInterface* WireframeMaterial;

public:
    UFUNCTION(BlueprintCallable)
    USceneObject* CreateNewSceneObject();

    UFUNCTION(BlueprintCallable)
    USceneObject* FindSceneObjectByActor(AActor* Actor);

    UFUNCTION(BlueprintCallable)
    bool DeleteSceneObject(USceneObject* Object);

    UFUNCTION(BlueprintCallable)
    bool DeleteSelectedSceneObjects();

    bool DeleteSelectedSceneObjects(AActor* SkipActor);


public:
    UFUNCTION(BlueprintCallable, Category = "UMeshSceneSubsystem")
    TArray<USceneObject*> GetSelection() const {
        return SelectedSceneObjects;
    }

    UFUNCTION(BlueprintCallable, Category = "UMeshSceneSubsystem")
    void ClearSelection();

    UFUNCTION(BlueprintCallable, Category = "UMeshSceneSubsystem")
    void SetSelected(USceneObject* SceneObject, bool bDeselect = false, bool bDeselectOthers = true);

    UFUNCTION(BlueprintCallable, Category = "UMeshSceneSubsystem")
    void ToggleSelected(USceneObject* SceneObject);

    UFUNCTION(BlueprintCallable, Category = "UMeshSceneSubsystem")
    void SetSelection(const TArray<USceneObject*>& SceneObjects);


    DECLARE_MULTICAST_DELEGATE_OneParam(FMeshSceneSelectionChangedEvent, UMeshSceneSubsystem*);
    FMeshSceneSelectionChangedEvent OnSelectionModified;



public:
    UFUNCTION(BlueprintCallable, Category = "UMeshSceneSubsystem")
    USceneObject* FindNearestHitObject(
        FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle,
        FVector& TriBaryCoords, float MaxDistance = 0
    );


protected:
    IToolsContextTransactionsAPI* TransactionsAPI = nullptr;

    UPROPERTY()
    TArray<USceneObject*> SceneObjects;

    void AddSceneObjectInternal(USceneObject* Object, bool bIsUndoRedo);
    void RemoveSceneObjectInternal(USceneObject* Object, bool bIsUndoRedo);

    UPROPERTY()
    TArray<USceneObject*> SelectedSceneObjects;

    void SetSelectionInternal(const TArray<USceneObject*>& SceneObjects);

    TUniquePtr<FMeshSceneSelectionChange> ActiveSelectionChange;
    void BeginSelectionChange();
    void EndSelectionChange();

    friend class FMeshSceneSelectionChange;
    friend class FAddRemoveSceneObjectChange;
};



/**
 * FMeshSelectionChange represents an reversible change to a UMeshSelectionSet
 */
class RUNTIMETOOLSSYSTEM_API FMeshSceneSelectionChange : public FToolCommandChange {
public:
    TArray<USceneObject*> OldSelection;
    TArray<USceneObject*> NewSelection;

    virtual void Apply(UObject* Object) override;
    virtual void Revert(UObject* Object) override;
    virtual FString ToString() const override {
        return TEXT("FMeshSceneSelectionChange");
    }
};



class RUNTIMETOOLSSYSTEM_API FAddRemoveSceneObjectChange : public FToolCommandChange {
public:
    USceneObject* SceneObject;
    bool bAdded = true;

public:
    virtual void Apply(UObject* Object) override;
    virtual void Revert(UObject* Object) override;
    virtual FString ToString() const override {
        return TEXT("FAddRemoveSceneObjectChange");
    }
};
