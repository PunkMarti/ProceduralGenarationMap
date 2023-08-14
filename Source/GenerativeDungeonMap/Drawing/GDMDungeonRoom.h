#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GDMDungeonRoom.generated.h"

/**
 *
 */
UCLASS()
class GENERATIVEDUNGEONMAP_API AGDMDungeonRoom : public AActor
{
	GENERATED_BODY()


		AGDMDungeonRoom(const FObjectInitializer& ObjectInitializer);

		virtual void BeginPlay()override;
		virtual void Tick(float DeltaTime)override;

	//Methods
public:

	void SetupRoomTransform(FTransform& _transform, FColor _ShapeColor,float _MeasureUnit,bool _draw = true);

	//Attributes
public:

	TObjectPtr<class UBoxComponent> BoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Room")
	TObjectPtr<class UStaticMeshComponent> FloorComponent;

};
