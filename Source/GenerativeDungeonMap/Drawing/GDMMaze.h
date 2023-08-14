#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GDMMaze.generated.h"

/**
 *
 */
UCLASS()
class GENERATIVEDUNGEONMAP_API AGDMMaze : public AActor
{
	GENERATED_BODY()


		AGDMMaze(const FObjectInitializer& ObjectInitializer);

		virtual void BeginPlay()override;
		virtual void Tick(float DeltaTime)override;
	//Methods
public:

	void InitMazeBody();
	/**  Process all the computation to fully create the maze */
	void GenerateMazeBody();
	/**  Process all the computation of one build step of the maze */
	void GenerateDynamicMaze();
	
	float GetMazeRealWidth();
	float GetMazeRealHeight();
	float GetRoomSize();
	FVector GetWallLocationFromCoord(int32 _x, int32 _y);
	FVector GetRoomLocationFromIndex(int32 _Index);
	
	void CreateNewWall(FTransform _WallTransform);
	void CreateHorizontalWall(int32 _i, int32 _j);
	void CreateVerticalWall(int32 _i, int32 _j);
	void CreateRooms();

	/** Reset all the computation */
	UFUNCTION(BlueprintCallable, Category = "Maze")
		void ResetMaze(bool _bInstantProcess, int32 _Width, int32 _Height);

	//Attributes
public:

	UPROPERTY(Category = "Maze", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UGDMAlgorithmComponent> MazeAlgorithmComponent;

	UPROPERTY(Category = "Maze", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UStaticMeshComponent> FloorMeshComponent;

	/** Sample wall for the maze walls */ 
	UPROPERTY(Category = "Maze", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UStaticMeshComponent> WallMeshComponent;

	UPROPERTY(Category = "Maze", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UTextRenderComponent> StartText;

	UPROPERTY(Category = "Maze", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UTextRenderComponent> TreasureText;

	UPROPERTY(Category = "Maze", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UTextRenderComponent> ExitText;

	/** Scale to apply to the wall defining the size of a room with RoomUnitSize */
	UPROPERTY(Category = "Maze", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float RoomScaleValue;

	/** unit room size for a scale of 1 */
	UPROPERTY(Category = "Maze", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float RoomUnitSize;

	UPROPERTY(Category = "Maze", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FVector OffSetText;

	/** Check if you want to see the maze built over time */
	UPROPERTY(Category = "Maze", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool bBuildOverTime;

};
