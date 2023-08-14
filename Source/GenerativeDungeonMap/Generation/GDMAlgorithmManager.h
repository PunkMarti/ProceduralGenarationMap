#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GDMAlgorithmManager.generated.h"

//UENUM(BlueprintType)
//enum class ETrainingRegistrationStatus : uint8
//{
//	Not_Participating,
//	NONE_Zero UMETA(Hidden),
//	Trainer,
//	Accepted,
//	Applied
//};

GENERATIVEDUNGEONMAP_API DECLARE_LOG_CATEGORY_EXTERN(LogAlgo, Log, All);

UENUM()
enum ECollisionType
{
	Touching,
	Overlap,
	Inside,
	Collision_None,
	Collision_MAX,
};

UENUM()
enum EBuildStep
{
	SeparateCells,
	Graph,
	UnitCells,
	OverlapRooms,
	OverlapCells,
	Finished,
	Build_None,
	Build_MAX,
};

USTRUCT(BlueprintType)
struct FCellData
{
	GENERATED_USTRUCT_BODY()

		FCellData() { }

		FCellData(FVector _Location, FVector _Size, int32 _ID = 0) {
		Size = _Size;
		Location = _Location;
		ID = _ID;
	}

		UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "CellData")
		FVector Size;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CellData")
		FVector Location;

	UPROPERTY(BlueprintReadWrite, Category = "CellData")
	int32 ID;

	bool operator==(const FCellData& _Cell)const
	{
		return ID == _Cell.ID;
	}

	bool operator!=(const FCellData& _Cell)const
	{
		return ID != _Cell.ID;
	}
};

USTRUCT(BlueprintType)
struct FCellOverlapData
{
	GENERATED_USTRUCT_BODY()

		FCellOverlapData() { }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CellData")
		FCellData Original;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CellData")
		TArray< FCellData> OverlapCells;

	UPROPERTY(BlueprintReadWrite, Category = "CellData")
		TArray<FCellData> UnitCells;

	TArray<FCellData> ProcessedHallways;

	TArray<FCellData> ProcessedUnitCells;

	bool operator==(const FCellOverlapData& _Cell)const
	{
		return Original == _Cell.Original;
	}

	bool operator!=(const FCellOverlapData& _Cell)const
	{
		return  Original != _Cell.Original;
	}

	bool operator==(const FCellData& _Cell)const
	{
		return Original == _Cell;
	}
};

USTRUCT(BlueprintType)
struct FEdgeData
{
	GENERATED_USTRUCT_BODY()
		FEdgeData() {}

	FEdgeData(FCellData* _A, FCellData* _B) {
		CellA = _A;
		CellB = _B;
	}

	FCellData* CellA;
	FCellData* CellB;
};

USTRUCT(BlueprintType)
struct FTriangleData
{
	GENERATED_USTRUCT_BODY()
		FTriangleData() {}

		FTriangleData(FCellData* _A, FCellData* _B, FCellData* _C) {
		CellA = _A;
		CellB = _B;
		CellC = _C;
	}

		FCellData* CellA;
	FCellData* CellB;
	FCellData* CellC;
};

USTRUCT(BlueprintType)
struct FGraphUnitData
{
	GENERATED_USTRUCT_BODY()
		FGraphUnitData() {}

	FGraphUnitData(FCellData* _Cell, TArray< FCellData*> _Neighbors) {
		Cell = _Cell;
		Neighbors = _Neighbors;
	}

	FCellData* Cell;
	TArray< FCellData*> Neighbors;

	bool operator==(const FGraphUnitData& _Other)const
	{
		return Cell == _Other.Cell;
	}

	bool operator==(const FCellData& _Other)const
	{
		return *Cell == _Other;
	}
};

/**
 * class managing the algorithm to generate a dungeon
 */
UCLASS()
class GENERATIVEDUNGEONMAP_API UGDMAlgorithmManager : public UActorComponent
{
	GENERATED_BODY()


	//Methods
public:

	void GenerateCells();

	void CompleteCellSeparation();
	bool SeparateAllCell();
	bool SeparateOneCell(FCellData& _cell);
	bool SeparateOneCellByIndex(int32 _Index);

	void ProcessMainRooms();

	void ProcessDelaunay();

	void ProcessGraph();
	void ProcessSimpleGraph();
	void ProcessCompleteSimpleGraph();
	void ProcessHallways();

	TArray<FCellData> ProcessOverlappingCellsByIndex(TArray<FCellData>& _ArrayToProcess, TArray<FCellData>& _ArrayToCompare,int32 _Idx);
	void BuildPathFromCell(int32 _CurrentIndex, TArray<FCellData*> _NewCells);

	void Reset();

	ECollisionType AreCellCollisioned(FCellData* _Cell1, FCellData* _Cell2);

	TArray<FCellData> ProcessUnitCellFromCell( FCellData& _Cell,float _Unit, int32 _ID);
	TArray<FCellData> ProcessSplitUnitCell(FCellData* _CellKeep, FCellData* _CellSplit);

	void AddHallway(FCellData _NewCell);
	void UnitTest();

	EBuildStep BuildStep();

	void BuildStepSeparateCells();
	void BuildStepGraph();
	void BuildStepUnitCells();
	void BuildStepOverlapRooms();
	void BuildStepOverlapCells();

	void CheckHallwaysToSplitIntoUnit();

	//Attributes
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Algorithm")
	int32 NumberOfCell;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Algorithm")
	FVector MinSizeOfCell;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Algorithm")
	FVector MaxSizeOfCell;

	TArray<FCellData> AllCells;

	TArray<FCellData> *Cells;

	TArray<FCellData*> MainRooms;
	TArray<FCellData*> HallRooms;

	TArray< FTriangleData> Triangles;
	TArray< FEdgeData> Edges;

	TArray< FGraphUnitData> Graph;
	TArray< FGraphUnitData> SimpleGraph;

	TArray<FCellData> Hallways;
	TArray<FCellData> HallwaysToUnitSplit;
	TArray<FCellData> ProcessedHallways;

	TArray<FCellData> UnitCells;
	TArray<FCellData> ProcessedUnitCells;
	TArray<FCellData> SplitedUnitCells;

	TArray<FCellOverlapData> OverlappingHallways;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Algorithm")
	TArray<FCellData> UnitTestCells;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Algorithm")
		FVector RandomChances;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Algorithm")
		FVector ThresholdMainRooms;

	/* Rand [0, RandChancesNewEdge) = 1 then we had an edge 1/RandChancesNewEdge chances for an edge to be added */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Algorithm")
		int32 RandChancesNewEdge;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Algorithm")
		float HallwayThreshold;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Algorithm")
		float CellCollisionThreshold;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Algorithm")
		bool bUnitTest;

	EBuildStep CurrentBuildStep;

	TMap<EBuildStep, int32> BuildStepIndex;
};
