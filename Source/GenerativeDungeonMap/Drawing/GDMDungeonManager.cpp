#include "GDMDungeonManager.h"
#include "GDMAlgorithmManager.h"
#include "GDMDungeonRoom.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

AGDMDungeonManager::AGDMDungeonManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	AlgorithmManager = CreateDefaultSubobject<UGDMAlgorithmManager>(TEXT("AlgoManager"));

	PrimaryActorTick.bCanEverTick = true;

	bDrawMainRooms = true;
	bDrawSmallRooms = true;
	bDrawHallRooms = true;
	bDrawHallways = true;
	bDrawProcessedHallways = true;
	bDrawFinal = false;
	bDrawUnitTest = false;
}

void AGDMDungeonManager::BeginPlay()
{
	Super::BeginPlay();

	InitDungeon();
}

void AGDMDungeonManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateDungeon(DeltaTime);
}

void AGDMDungeonManager::InitDungeon()
{
	if (nullptr != AlgorithmManager)
	{
		AlgorithmManager->GenerateCells();
		UpdateAllRooms();
	}
}

void AGDMDungeonManager::UpdateDungeon(float DeltaTime)
{
	if (bBuildOverTime && !bManualBuild)
	{
		BuildStep();
	}
}

void AGDMDungeonManager::ResetDungeon()
{
	if (nullptr != AlgorithmManager)
	{
		AlgorithmManager->Reset();
	}

	bBuildOverTime = true;

	for (auto room : AllRooms)
		room->Destroy();

	for (auto room : Hallways)
		room->Destroy();

	for (auto room : UnitCells)
		room->Destroy();
	
	for (auto room : ProcessedUnitCells)
		room->Destroy();

	for (auto room : SplitedUnitCells)
		room->Destroy();

	for (auto room : ProcessedHallways)
		room->Destroy();

	Hallways.Empty();
	AllRooms.Empty();
	UnitCells.Empty();
	ProcessedUnitCells.Empty();
	SplitedUnitCells.Empty();
	ProcessedHallways.Empty();

	InitDungeon();
}

void AGDMDungeonManager::BuildStep()
{
	ProcessDungeon();
	UpdateDrawByState();
}

void AGDMDungeonManager::ProcessDelaunay()
{
	AlgorithmManager->ProcessDelaunay();
}

void AGDMDungeonManager::ProcessGraph()
{
	AlgorithmManager->ProcessGraph();
}

void AGDMDungeonManager::ProcessSimpleGraph()
{
	AlgorithmManager->ProcessSimpleGraph();
}

void AGDMDungeonManager::ProcessCompleteSimpleGraph()
{
	AlgorithmManager->ProcessCompleteSimpleGraph();
}

void AGDMDungeonManager::ProcessHallways()
{
	AlgorithmManager->ProcessHallways();
}

void AGDMDungeonManager::UpdateDraw(bool _bDrawMainRooms, bool _bDrawSmallRooms, bool _bDrawHallRooms, bool _bDrawHallways, bool _bDrawProcessedHallways,
	bool _bDrawFinal, bool _bDrawUnitCells, bool _bDrawProcessedUnitCells, bool _bDrawSplitedUnitCells,
	bool _bDrawUnitTest)
{
	bDrawMainRooms = _bDrawMainRooms;
	bDrawSmallRooms = _bDrawSmallRooms;
	bDrawHallRooms = _bDrawHallRooms;
	bDrawHallways = _bDrawHallways;
	bDrawProcessedHallways = _bDrawProcessedHallways;
	bDrawFinal = _bDrawFinal;
	bDrawUnitCells = _bDrawUnitCells;
	bDrawProcessedUnitCells = _bDrawProcessedUnitCells;
	bDrawSplitedUnitCells = _bDrawSplitedUnitCells;
	bDrawUnitTest = _bDrawUnitTest;
	if (bDrawFinal)
	{
		bDrawMainRooms = true;
		bDrawSmallRooms = false;
		bDrawHallRooms = true;
		bDrawHallways = false;
		bDrawProcessedHallways = true;
		bDrawProcessedUnitCells = false;
		bDrawSplitedUnitCells = true;
		bDrawUnitCells = false;
		bDrawUnitTest = false;
	}
	DrawMainRooms(bDrawMainRooms);
	DrawSmallRooms(bDrawSmallRooms);
	DrawHallRooms(bDrawHallRooms);

	DrawHallways(bDrawHallways);
	DrawProcessHallways(bDrawProcessedHallways);
	DrawUnitCells(bDrawUnitCells);
	DrawProcessedUnitCells(bDrawProcessedUnitCells);
	DrawSplitedUnitCells(bDrawSplitedUnitCells);
	DrawArrayCell(AlgorithmManager->UnitTestCells, UnitTestCells, FColor::Cyan, bDrawUnitTest);
}

void AGDMDungeonManager::UpdateDrawByState()
{

	switch (AlgorithmManager->CurrentBuildStep)
	{
	case EBuildStep::SeparateCells:
		DrawMainRooms(bDrawMainRooms);
		DrawSmallRooms(bDrawSmallRooms);
		break;

	case EBuildStep::Graph:
		DrawMainRooms(bDrawMainRooms);
		DrawSmallRooms(bDrawSmallRooms);
		DrawHallways(bDrawHallways);
		break;

	case EBuildStep::UnitCells:
		DrawMainRooms(bDrawMainRooms);
		DrawUnitCells(bDrawUnitCells);
		break;

	case EBuildStep::OverlapRooms:
		DrawProcessedUnitCells(bDrawProcessedUnitCells);
		DrawHallRooms(bDrawHallRooms);
		DrawSplitedUnitCells(bDrawSplitedUnitCells);
		break;

	case EBuildStep::OverlapCells:
		DrawProcessHallways(bDrawProcessedHallways);
		DrawSplitedUnitCells(bDrawSplitedUnitCells);
		break;
	}
}

void AGDMDungeonManager::DrawMainRooms(bool _draw)
{
	if (nullptr != AlgorithmManager)
	{
		FTransform NewTransform;
		int32 Idx = 0;
		for (int32 i = 0; i < AlgorithmManager->Cells->Num(); ++i)
		{
			FCellData cell = (*AlgorithmManager->Cells)[i];

			NewTransform.SetLocation(cell.Location * MeasureUnit);
			NewTransform.SetScale3D(cell.Size * MeasureUnit);

			if (AlgorithmManager->MainRooms.FindByPredicate([&cell](const FCellData* _Data) {
				return *_Data == cell; }) != nullptr)
			{
				UpdateRoomCell(AllRooms, i, NewTransform, FColor::Yellow, _draw);
				Idx++;
			}
		}
	}
}

void AGDMDungeonManager::DrawSmallRooms(bool _draw)
{
	if (nullptr != AlgorithmManager)
	{
		FTransform NewTransform;
		int32 Idx = 0;
		for (int32 i = 0; i < AlgorithmManager->Cells->Num(); ++i)
		{
			FCellData cell = (*AlgorithmManager->Cells)[i];

			NewTransform.SetLocation(cell.Location * MeasureUnit);
			NewTransform.SetScale3D(cell.Size * MeasureUnit);

			if (AlgorithmManager->MainRooms.FindByPredicate([&cell](const FCellData* _Data) {
				return *_Data == cell; }) == nullptr &&
				AlgorithmManager->HallRooms.FindByPredicate([&cell](const FCellData* _Data) {
					return *_Data == cell; }) == nullptr)
			{
				UpdateRoomCell(AllRooms, i, NewTransform, FColor::Red, _draw);

				Idx++;
			}
		}
	}
}

void AGDMDungeonManager::DrawHallRooms(bool _draw)
{
	if (nullptr != AlgorithmManager)
	{
		FTransform NewTransform;
		int32 Idx = 0;
		for (int32 i = 0; i < AlgorithmManager->Cells->Num(); ++i)
		{
			FCellData cell = (*AlgorithmManager->Cells)[i];

			NewTransform.SetLocation(cell.Location * MeasureUnit);
			NewTransform.SetScale3D(cell.Size * MeasureUnit);

			if (AlgorithmManager->HallRooms.FindByPredicate([&cell](const FCellData* _Data) {
				return *_Data == cell; }) != nullptr)
			{
				UpdateRoomCell(AllRooms, i, NewTransform, FColor::Cyan, _draw);

				Idx++;
			}
		}
	}
}

void AGDMDungeonManager::DrawHallways(bool _draw)
{
	if (nullptr != AlgorithmManager)
	{
		FTransform NewTransform;
		for (int32 i = 0; i < AlgorithmManager->Hallways.Num(); ++i)
		{
			FCellData cell = AlgorithmManager->Hallways[i];
			NewTransform.SetLocation(cell.Location * MeasureUnit);
			NewTransform.SetScale3D(cell.Size * MeasureUnit);
			UpdateRoomCell(Hallways, i, NewTransform, FColor::Blue, _draw);

		}
	}
}

void AGDMDungeonManager::DrawProcessHallways(bool _draw)
{
	if (nullptr != AlgorithmManager)
	{
		FTransform NewTransform;
		int32 j = 0;
		for (int32 i = AlgorithmManager->BuildStepIndex[EBuildStep::OverlapCells] > 0 ? AlgorithmManager->ProcessedHallways.Num() -1 : 0; i < AlgorithmManager->ProcessedHallways.Num() && i >= 0; ++i)
		{
			FCellData cell = AlgorithmManager->ProcessedHallways[i];
			NewTransform.SetLocation(cell.Location * MeasureUnit);
			NewTransform.SetScale3D(cell.Size * MeasureUnit);
			UpdateRoomCell(ProcessedHallways, i, NewTransform, FColor::Cyan, _draw);
			j = i;
		}

		for (auto& Hall : AlgorithmManager->OverlappingHallways)
		{
			for (auto cell : Hall.ProcessedHallways)
			{
				j++;
				NewTransform.SetLocation(cell.Location * MeasureUnit);
				NewTransform.SetScale3D(cell.Size * MeasureUnit);
				UpdateRoomCell(ProcessedHallways, j, NewTransform, FColor::Red, _draw);
			}
		}
	}
}

void AGDMDungeonManager::DrawUnitCells(bool _draw)
{
	if (nullptr != AlgorithmManager)
	{
		FTransform NewTransform;

		for (int32 i = AlgorithmManager->BuildStepIndex[EBuildStep::UnitCells] > 0 ? AlgorithmManager->BuildStepIndex[EBuildStep::UnitCells] - 1 : 0; i < AlgorithmManager->UnitCells.Num() ; ++i)
		{
			FCellData cell = AlgorithmManager->UnitCells[i];
			NewTransform.SetLocation(cell.Location * MeasureUnit);
			NewTransform.SetScale3D(cell.Size * MeasureUnit);
			UpdateRoomCell(UnitCells,i, NewTransform, FColor::Purple, _draw);
		}
	}
}

void AGDMDungeonManager::DrawProcessedUnitCells(bool _draw)
{
	if (nullptr != AlgorithmManager)
	{
		FTransform NewTransform;
		int32 j = 0;
		for (int32 i = AlgorithmManager->BuildStepIndex[EBuildStep::OverlapRooms] > 0 ? AlgorithmManager->BuildStepIndex[EBuildStep::OverlapRooms] - 1 : 0; i < AlgorithmManager->ProcessedUnitCells.Num(); ++i)
		{
			FCellData cell = AlgorithmManager->ProcessedUnitCells[i];
			NewTransform.SetLocation(cell.Location * MeasureUnit);
			NewTransform.SetScale3D(cell.Size * MeasureUnit);
			UpdateRoomCell(ProcessedUnitCells, i, NewTransform, FColor::Red, _draw);
			j = i;
		}
		
		for (auto& Hall : AlgorithmManager->OverlappingHallways)
		{
			for (auto cell : Hall.ProcessedUnitCells)
			{
				j++;
				NewTransform.SetLocation(cell.Location * MeasureUnit);
				NewTransform.SetScale3D(cell.Size * MeasureUnit);
				UpdateRoomCell(ProcessedUnitCells, j, NewTransform, FColor::Red, _draw);
			}
		}
	}
}

void AGDMDungeonManager::DrawSplitedUnitCells(bool _draw)
{
	if (nullptr != AlgorithmManager)
	{
		FTransform NewTransform;

		for (int32 i = 0; i < AlgorithmManager->SplitedUnitCells.Num(); ++i)
		{
			FCellData cell = AlgorithmManager->SplitedUnitCells[i];
			NewTransform.SetLocation(cell.Location * MeasureUnit);
			NewTransform.SetScale3D(cell.Size * MeasureUnit);
			UpdateRoomCell(SplitedUnitCells, i, NewTransform, FColor::Green, _draw);
		}
	}
}

void AGDMDungeonManager::DrawFinal(bool _draw)
{
	DrawMainRooms(bDrawMainRooms);
	DrawSmallRooms(bDrawSmallRooms);
	DrawHallRooms(bDrawHallRooms);

	DrawHallways(bDrawHallways);
	DrawProcessHallways(bDrawProcessedHallways);
}

void AGDMDungeonManager::DrawArrayCell(TArray<FCellData>& _Array, TArray< TObjectPtr<class AGDMDungeonRoom>>& _ArrayRoom, FColor _Color, bool _draw)
{
	if (nullptr != AlgorithmManager)
	{
		FTransform NewTransform;

		for (int32 i = 0; i < _Array.Num(); ++i)
		{
			FCellData cell = _Array[i];
			NewTransform.SetLocation(cell.Location * MeasureUnit);
			NewTransform.SetScale3D(cell.Size * MeasureUnit);
			UpdateRoomCell(_ArrayRoom, i, NewTransform, _Color, _draw);
		}
	}
}

void AGDMDungeonManager::DrawDelaunay()
{
	if (nullptr != AlgorithmManager)
	{
		for (auto Tri : AlgorithmManager->Triangles)
		{
			DrawDebugLine(GetWorld(), Tri.CellA->Location * MeasureUnit, Tri.CellB->Location * MeasureUnit, FColor::Green, false,5.f);
			DrawDebugLine(GetWorld(), Tri.CellA->Location * MeasureUnit, Tri.CellC->Location * MeasureUnit, FColor::Green, false, 5.f);
			DrawDebugLine(GetWorld(), Tri.CellC->Location * MeasureUnit, Tri.CellB->Location * MeasureUnit, FColor::Green, false, 5.f);
		}

		for (auto Tri : AlgorithmManager->Edges)
		{
			DrawDebugLine(GetWorld(), Tri.CellA->Location * MeasureUnit, Tri.CellB->Location * MeasureUnit, FColor::Purple, false, 5.f);
		}
	}
}

void AGDMDungeonManager::DrawGraph()
{
	if (nullptr != AlgorithmManager)
	{
		for (auto Tri : AlgorithmManager->Graph)
		{
			for (auto neighb : Tri.Neighbors)
			{
				DrawDebugLine(GetWorld(), Tri.Cell->Location * MeasureUnit, neighb->Location * MeasureUnit, FColor::Red, false, 5.f);
			}
		}
	}
}

void AGDMDungeonManager::DrawSimpleGraph()
{
	if (nullptr != AlgorithmManager)
	{
		for (auto Tri : AlgorithmManager->SimpleGraph)
		{
			for (auto neighb : Tri.Neighbors)
			{
				DrawDebugLine(GetWorld(), Tri.Cell->Location * MeasureUnit, neighb->Location * MeasureUnit, FColor::Blue, false, 5.f);
			}
		}
	}
}

void AGDMDungeonManager::ProcessAllDungeon()
{
	AlgorithmManager->CompleteCellSeparation();
}

void AGDMDungeonManager::ProcessDungeon()
{
	if (AlgorithmManager->BuildStep() == EBuildStep::Finished)
	{
		bBuildOverTime = false;
		UpdateDraw(bDrawMainRooms, bDrawSmallRooms, bDrawHallRooms, bDrawHallways, bDrawProcessedHallways, bDrawFinal, bDrawUnitCells, bDrawProcessedUnitCells, bDrawSplitedUnitCells, bDrawUnitTest);
	}
}

void AGDMDungeonManager::UpdateAllRooms()
{
	if (nullptr != AlgorithmManager)
	{
		DrawMainRooms(bDrawMainRooms);

		DrawSmallRooms(bDrawSmallRooms);

		DrawHallRooms(bDrawHallRooms);
	}
}

void AGDMDungeonManager::UpdateRoomCell(TArray< TObjectPtr<class AGDMDungeonRoom>>& _Rooms, int32 _Index, FTransform& _Transform, FColor _Color, bool _draw)
{
	if (_Index < _Rooms.Num())
	{
		if (nullptr != _Rooms[_Index])
		{
			_Rooms[_Index]->SetupRoomTransform(_Transform, _Color, MeasureUnit, _draw);
		}
		else
		{
			AGDMDungeonRoom* NewRoom = Cast< AGDMDungeonRoom>(GetWorld()->SpawnActor< AGDMDungeonRoom>(RoomClass, _Transform));

			if (nullptr != NewRoom)
			{
				NewRoom->SetupRoomTransform(_Transform, _Color, MeasureUnit, _draw);
				_Rooms[_Index] = NewRoom;
			}
		}
	}
	else
	{
		AGDMDungeonRoom* NewRoom = Cast< AGDMDungeonRoom>(GetWorld()->SpawnActor< AGDMDungeonRoom>(RoomClass, _Transform));

		if (nullptr != NewRoom)
		{
			NewRoom->SetupRoomTransform(_Transform, _Color, MeasureUnit, _draw);
			_Rooms.Add(NewRoom);
		}
	}
}