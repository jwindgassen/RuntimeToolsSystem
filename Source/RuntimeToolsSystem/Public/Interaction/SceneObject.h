#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/DynamicMeshAABBTree3.h"
#include "SceneObject.generated.h"

struct FMeshDescription;

/**
 * USceneObject is a "Scene Object" in the "Scene". Do not create these yourself.
 * Use the functions in UInteractionComponent to create and manage SceneObjects.
 *
 * Conceptually, USceneObject is a triangle mesh object that can be selected,
 * transformed, and edited using mesh editing tools.
 *
 * Under the hood, USceneObject will spawn a ADynamicSDMCActor to actually implement
 * most of that functionality. But, the premise is that the higher level Scene is not aware
 * of those details.
 */
UCLASS()
class RUNTIMETOOLSSYSTEM_API USceneObject : public UObject {
    using FDynamicMesh3 = UE::Geometry::FDynamicMesh3;
    using FDynamicMeshAABBTree3 = UE::Geometry::FDynamicMeshAABBTree3;

    GENERATED_BODY()

public:
    USceneObject();

    void Initialize(UWorld* TargetWorld, const FMeshDescription* InitialMeshDescription);
    void Initialize(UWorld* TargetWorld, const FDynamicMesh3* InitialMesh);

    // set the 3D transform of this SceneObject
    void SetTransform(FTransform Transform);

    // get the Actor that represents this SceneObject
    AActor* GetActor();

    // get the mesh component that represents this SceneObject
    UMeshComponent* GetMeshComponent();


    //
    // Material functions
    //

    UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
    void CopyMaterialsFromComponent();

    UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
    void SetAllMaterials(UMaterialInterface* SetToMaterial);

    UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
    void SetToHighlightMaterial(UMaterialInterface* Material);

    UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
    void ClearHighlightMaterial();


    //
    // Spatial Query functions
    //

    UFUNCTION(BlueprintCallable, Category = "RuntimeMeshSceneObject")
    bool IntersectRay(
        FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle,
        FVector& TriBaryCoords, float MaxDistance = 0
    );

    // USceneObject's representation in UE Level is a AActor
    UPROPERTY()
    AActor* Actor = nullptr;

protected:
    TUniquePtr<FDynamicMesh3> SourceMesh;
    TUniquePtr<FDynamicMeshAABBTree3> MeshAABBTree;

    void UpdateSourceMesh(const FMeshDescription* MeshDescription);

    TArray<UMaterialInterface*> Materials;
    void UpdateComponentMaterials(bool bForceRefresh);
};
