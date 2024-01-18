#include "Interaction/SceneObject.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/DynamicMeshAABBTree3.h"
#include "MeshDescriptionToDynamicMesh.h"
#include "MaterialDomain.h"
#include "Materials/Material.h"

using namespace UE::Geometry;

USceneObject::USceneObject() {
    if (!SourceMesh) {
        SourceMesh = MakeUnique<FDynamicMesh3>();
    }
    if (!MeshAABBTree) {
        MeshAABBTree = MakeUnique<FDynamicMeshAABBTree3>();
    }

    UMaterialInterface* DefaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
    Materials.Add(DefaultMaterial);
}


void USceneObject::Initialize(UWorld* TargetWorld, const FMeshDescription* InitialMeshDescription) {
    FActorSpawnParameters SpawnInfo;
    Actor = TargetWorld->SpawnActor<AActor>(FVector::ZeroVector, FRotator(0, 0, 0), SpawnInfo);

    UpdateSourceMesh(InitialMeshDescription);
    UpdateComponentMaterials(false);
}

void USceneObject::Initialize(UWorld* TargetWorld, const FDynamicMesh3* InitialMesh) {
    FActorSpawnParameters SpawnInfo;
    Actor = TargetWorld->SpawnActor<AActor>(FVector::ZeroVector, FRotator(0, 0, 0), SpawnInfo);

    *SourceMesh = *InitialMesh;
    MeshAABBTree->SetMesh(SourceMesh.Get(), true);

    UpdateComponentMaterials(false);
}


void USceneObject::SetTransform(FTransform Transform) {
    GetActor()->SetActorTransform(Transform);
}


AActor* USceneObject::GetActor() {
    return Actor;
}

UMeshComponent* USceneObject::GetMeshComponent() {
    return Actor->FindComponentByClass<UMeshComponent>();
}



void USceneObject::CopyMaterialsFromComponent() {
    UMeshComponent* Component = GetMeshComponent();
    int32 NumMaterials = Component->GetNumMaterials();
    if (NumMaterials == 0) {
        Materials = {UMaterial::GetDefaultMaterial(MD_Surface)};
    } else {
        Materials.SetNum(NumMaterials);
        for (int32 k = 0; k < NumMaterials; ++k) {
            Materials[k] = Component->GetMaterial(k);
        }
    }
}


void USceneObject::SetAllMaterials(UMaterialInterface* SetToMaterial) {
    int32 NumMaterials = Materials.Num();
    for (int32 k = 0; k < NumMaterials; ++k) {
        Materials[k] = SetToMaterial;
    }
    UpdateComponentMaterials(true);
}


void USceneObject::SetToHighlightMaterial(UMaterialInterface* Material) {
    UMeshComponent* Component = GetMeshComponent();
    int32 NumMaterials = FMath::Max(1, Component->GetNumMaterials());
    for (int32 k = 0; k < NumMaterials; ++k) {
        Component->SetMaterial(k, Material);
    }
}

void USceneObject::ClearHighlightMaterial() {
    UpdateComponentMaterials(true);
}


void USceneObject::UpdateComponentMaterials(bool bForceRefresh) {
    UMaterialInterface* DefaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);

    UMeshComponent* Component = GetMeshComponent();
    if (!Component) {
        return;
    }

    int32 NumMaterials = FMath::Max(1, Component->GetNumMaterials());
    for (int32 k = 0; k < NumMaterials; ++k) {
        UMaterialInterface* SetMaterial = (k < Materials.Num()) ? Materials[k] : DefaultMaterial;
        Component->SetMaterial(k, SetMaterial);
    }
}



void USceneObject::UpdateSourceMesh(const FMeshDescription* MeshDescriptionIn) {
    FMeshDescriptionToDynamicMesh Converter;
    FDynamicMesh3 TmpMesh;
    Converter.Convert(MeshDescriptionIn, TmpMesh);
    *SourceMesh = MoveTemp(TmpMesh);

    MeshAABBTree->SetMesh(SourceMesh.Get(), true);
}



bool USceneObject::IntersectRay(
    FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle,
    FVector& TriBaryCoords, float MaxDistance
) {
    if (!ensure(SourceMesh)) {
        return false;
    }
    if (!GetActor()) {
        return false;  // this can happen if the Actor gets deleted but the SO does not. Bad situation but avoids
                       // crash...
    }

    FTransformSRT3d ActorToWorld(GetActor()->GetActorTransform());
    FVector3d WorldDirection(RayDirection);
    WorldDirection.Normalize();
    FRay3d LocalRay(
        ActorToWorld.InverseTransformPosition(RayOrigin), ActorToWorld.InverseTransformNormal(WorldDirection)
    );
    IMeshSpatial::FQueryOptions QueryOptions;
    if (MaxDistance > 0) {
        QueryOptions.MaxDistance = MaxDistance;
    }
    NearestTriangle = MeshAABBTree->FindNearestHitTriangle(LocalRay, QueryOptions);
    if (SourceMesh->IsTriangle(NearestTriangle)) {
        FIntrRay3Triangle3d IntrQuery =
            TMeshQueries<FDynamicMesh3>::TriangleIntersection(*SourceMesh, NearestTriangle, LocalRay);
        if (IntrQuery.IntersectionType == EIntersectionType::Point) {
            HitDistance = IntrQuery.RayParameter;
            WorldHitPoint = ActorToWorld.TransformPosition(LocalRay.PointAt(IntrQuery.RayParameter));
            TriBaryCoords = IntrQuery.TriangleBaryCoords;
            return true;
        }
    }
    return false;
}
