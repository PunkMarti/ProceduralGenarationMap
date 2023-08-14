#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GDMDungeonManager.generated.h"

struct FCellData;

/**
 *
 */
UCLASS()
class GENERATIVEDUNGEONMAP_API AGDMDungeonManager : public AActor
{
	GENERATED_BODY()


		AGDMDungeonManager(const FObjectInitializer& ObjectInitializer);

		virtual void BeginPlay()override;
		virtual void Tick(float DeltaTime)override;

	//Methods
public:

	void InitDungeon();
	void UpdateDungeon(float DeltaTime);
	
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
		void ResetDungeon();
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void BuildStep();
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
		void ProcessDelaunay();
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
		void ProcessGraph();
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
		void ProcessSimpleGraph();
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
		void ProcessCompleteSimpleGraph();
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
		void ProcessHallways();

	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void UpdateDraw(bool _bDrawMainRooms, bool _bDrawSmallRooms, bool _bDrawHallRooms, bool _bDrawHallways, bool _bDrawProcessedHallways, bool _bDrawFinal, bool _bDrawUnitCells,bool _bDrawProcessedUnitCells, bool _bDrawSplitedUnitCells, bool _bDrawUnitTest);

	void UpdateDrawByState();

	void DrawMainRooms(bool _draw);
	void DrawSmallRooms(bool _draw);
	void DrawHallRooms(bool _draw);

	void DrawHallways(bool _draw);
	void DrawProcessHallways(bool _draw);
	void DrawUnitCells(bool _draw);
	void DrawProcessedUnitCells(bool _draw);
	void DrawSplitedUnitCells(bool _draw);
	void DrawArrayCell(TArray<FCellData>& _Array, TArray< TObjectPtr<class AGDMDungeonRoom>>& _ArrayRoom,FColor _Color,bool _draw);
	void DrawFinal(bool _draw);
	
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void DrawDelaunay();
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void DrawGraph();
	UFUNCTION(BlueprintCallable, Category = "Dungeon")
	void DrawSimpleGraph();

	void ProcessAllDungeon();
	void ProcessDungeon();

	void UpdateAllRooms();
	void UpdateRoomCell(TArray< TObjectPtr<class AGDMDungeonRoom>>& _Rooms, int32 _Index, FTransform& _Transform, FColor _Color, bool _draw = true);

	//Attributes
public:

	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UGDMAlgorithmManager> AlgorithmManager;

	/** unit room size for a scale of 1 */
	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float MeasureUnit;

	/** Check if you want to see the maze built over time */
	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool bBuildOverTime;

	/** Check if you want to see the maze built over time */
	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool bManualBuild;
	
	TArray< TObjectPtr<class AGDMDungeonRoom>> AllRooms;
	TArray< TObjectPtr<class AGDMDungeonRoom>> MainRooms;
	TArray< TObjectPtr<class AGDMDungeonRoom>> SmallRooms;
	TArray< TObjectPtr<class AGDMDungeonRoom>> HallRooms;
	TArray< TObjectPtr<class AGDMDungeonRoom>> Hallways;
	TArray< TObjectPtr<class AGDMDungeonRoom>> ProcessedHallways;
	TArray< TObjectPtr<class AGDMDungeonRoom>> UnitCells;
	TArray< TObjectPtr<class AGDMDungeonRoom>> ProcessedUnitCells;
	TArray< TObjectPtr<class AGDMDungeonRoom>> SplitedUnitCells;
	TArray< TObjectPtr<class AGDMDungeonRoom>> UnitTestCells;

	/** Check if you want to see the maze built over time */
	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bDrawMainRooms;
	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bDrawSmallRooms;
	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bDrawHallRooms;
	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bDrawHallways;
	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bDrawProcessedHallways;
	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bDrawUnitCells;
	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bDrawProcessedUnitCells;
	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bDrawSplitedUnitCells;
	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bDrawUnitTest;
	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bDrawFinal;

	UPROPERTY(Category = "Dungeon", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TSubclassOf< AGDMDungeonRoom> RoomClass;

};
