// Fill out your copyright notice in the Description page of Project Settings.

#include "PointCloudBase.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"


#include <cstdlib>
#include "libqhullcpp/Qhull.h"
#include "libqhullcpp/QhullFacetList.h"
#include "libqhullcpp/QhullVertexSet.h"
#include "libqhullcpp/QhullVertex.h"
#include "libqhullcpp/QhullPoint.h"

using namespace orgQhull;

// Sets default values
APointCloudBase::APointCloudBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Game/Geometry/Sphere"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialPoint(TEXT("/Game/Materials/Point"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialHull(TEXT("/Game/Materials/Hull"));

    m_SphereRadius = 200.0f;
    m_PointCount = 2000;

    PointMDI = UMaterialInstanceDynamic::Create(MaterialPoint.Object, this, TEXT("PointMDI"));
    HullMDI = UMaterialInstanceDynamic::Create(MaterialHull.Object, this, TEXT("HullMDI"));

    InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedStaticMeshComponent"));
    InstancedStaticMeshComponent->SetupAttachment(RootComponent);
    InstancedStaticMeshComponent->SetStaticMesh(Sphere.Object);

    ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMeshComponent"));
    ProceduralMeshComponent->SetupAttachment(InstancedStaticMeshComponent);
}

// Called when the game starts or when spawned
void APointCloudBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APointCloudBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APointCloudBase::GeneratePointCloud()
{
    if (InstancedStaticMeshComponent->GetInstanceCount()) {
        InstancedStaticMeshComponent->ClearInstances();
    }
    if (ProceduralMeshComponent->GetProcMeshSection(0)) {
        ProceduralMeshComponent->ClearMeshSection(0);
    }
    
    FTransform transform = {};
    transform.SetScale3D(FVector(0.05f, 0.05f, 0.05f));

    static int seed = 0;
    std::srand(seed++);

    for (uint32 i = 0; i < m_PointCount; ++i) {
        transform.SetLocation((m_SphereRadius / RAND_MAX) * FVector((std::rand() << 1) - RAND_MAX, (std::rand() << 1) - RAND_MAX, (std::rand() << 1) - RAND_MAX));
        InstancedStaticMeshComponent->AddInstance(transform);
    }
    PointMDI->SetScalarParameterValue("Radius", m_SphereRadius);
    PointMDI->SetVectorParameterValue("CloudOffset", GetTransform().GetLocation());
    InstancedStaticMeshComponent->SetMaterial(0, PointMDI);
}

void APointCloudBase::GenerateConvexHull()
{
    const auto instanceCount = InstancedStaticMeshComponent->GetInstanceCount();
    if (!instanceCount) {
        return;
    }

    if (ProceduralMeshComponent->GetProcMeshSection(0)) {
        ProceduralMeshComponent->ClearMeshSection(0);
    }

    std::vector<double> points;
    points.reserve(instanceCount * 3);

    FTransform transform;
    for (auto i = 0; i < instanceCount; ++i) {
        InstancedStaticMeshComponent->GetInstanceTransform(i, transform);
        const auto location = transform.GetLocation();
        points.push_back(location.X);
        points.push_back(location.Y);
        points.push_back(location.Z);
    }

    Qhull qhull("", 3, instanceCount, points.data(), "Qt");

    const auto facets = qhull.facetList().toStdVector();

    TArray<FVector> vertices;
    vertices.Reserve(facets.size() * 3);

    for (const auto& facet : facets) {
        for (const auto& vertex : facet.vertices().toStdVector()) {
            const double* point = vertex.point().coordinates();
            vertices.Add(FVector(point[0], point[1], point[2]));
        }
    }

    TArray<FLinearColor> vertexColors;
    vertexColors.Init(FLinearColor(0.00, 0.5, 0.00, 0.1), vertices.Num());

    TArray<int32> indices;
    indices.Reserve(vertices.Num());
    for (int i = 0; i < vertices.Num(); ++i) {
        indices.Add(i);
    }

    HullMDI->SetScalarParameterValue("Radius", m_SphereRadius);
    ProceduralMeshComponent->CreateMeshSection_LinearColor(0, vertices, indices, {}, {}, vertexColors, {}, false);
    ProceduralMeshComponent->SetMaterial(0, HullMDI);
}

