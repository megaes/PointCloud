// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PointCloudBase.generated.h"

UCLASS()
class POINTCLOUD_API APointCloudBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APointCloudBase();

protected:
    UPROPERTY(EditDefaultsOnly, Category = PointCloud, meta = (ClampMin = "10.0", ClampMax = "100.0", UIMin = "10.0", UIMax = "100.0", DisplayName = "SphereRadius"))
    float m_SphereRadius;

    UPROPERTY(EditDefaultsOnly, Category = PointCloud, meta = (ClampMin = "1000.0", ClampMax = "5000.0", UIMin = "1000.0", UIMax = "5000.0", DisplayName = "PointCount"))
    uint32 m_PointCount;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable)
    void GeneratePointCloud();

    UFUNCTION(BlueprintCallable)
    void GenerateConvexHull();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:	
    class UInstancedStaticMeshComponent*    InstancedStaticMeshComponent;
    class UProceduralMeshComponent*         ProceduralMeshComponent;

    UPROPERTY(VisibleInstanceOnly)
    class UMaterialInstanceDynamic*         PointMDI;

    UPROPERTY(VisibleInstanceOnly)
    class UMaterialInstanceDynamic*         HullMDI;
};
