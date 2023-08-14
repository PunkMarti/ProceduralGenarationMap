#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GDMAlgorithmComponent.generated.h"

/**
 *
 */
UCLASS()
class GENERATIVEDUNGEONMAP_API UGDMAlgorithmComponent : public UActorComponent
{
	GENERATED_BODY()


	//Methods
public:

	void InitMaze();
	/** Process all walls placement */
	void GenerateMaze();
	/** Process one wall placement */
	bool PlaceWall(int32& _xH, int32& _yH, int32& _xV, int32& _yV);

	/** Random the rooms */
	void GenerateRooms();

	void ResetMaze(int32 _Width, int32 _Height);

	FORCEINLINE bool IsGenerationOver()
	{
		return CurrentFailAttemptNumber > GetMaxPlacingWallFailAttempt();
	}

	FORCEINLINE uint8 GetNumberOfWidthWall()
	{
		return (Width * 2) + 1;
	}

	FORCEINLINE	uint8 GetNumberOfHeightWall()
	{
		return (Height * 2) + 1;
	}

	FORCEINLINE uint8 GetNumberOfHorizontalWidthWall()
	{
		return Width;
	}

	FORCEINLINE	uint8 GetNumberOfHorizontalHeightWall()
	{
		return Height + 1;
	}

	FORCEINLINE uint8 GetNumberOfVerticalWidthWall()
	{
		return Width + 1;
	}

	FORCEINLINE	uint8 GetNumberOfVerticalHeightWall()
	{
		return Height ;
	}


	FORCEINLINE uint8 GetWall(int32 _x, int32 _y) {
		return Walls[GetIndexWall(_x,_y)];
	}

	FORCEINLINE int32 GetIndexWall(int32 _x, int32 _y) {
		return (((_y/2) + (_y%2)) * GetNumberOfHorizontalWidthWall()) 
			+ ((_y/2) * GetNumberOfVerticalWidthWall()) 
			+ _x;
	}

	/** Max number of attempt before stopping the computation */
	FORCEINLINE	int32 GetMaxPlacingWallFailAttempt()
	{
		return Width * Height;
	}

	//Attributes
public:

	/** Number of room on a line */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Algorithm")
	uint8 Width;

	/** Number of room in a column */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Algorithm")
	uint8 Height;

	TArray<uint8> Walls;

	int32 StartIndex;
	int32 TreasureIndex;
	int32 ExitIndex;

	int32 CurrentFailAttemptNumber;
};
