#include "GDMAlgorithmManager.h"

DEFINE_LOG_CATEGORY(LogAlgo);

void UGDMAlgorithmManager::GenerateCells()
{

	BuildStepIndex.Add(EBuildStep::SeparateCells, 0);
	BuildStepIndex.Add(EBuildStep::Graph, 0);
	BuildStepIndex.Add(EBuildStep::UnitCells, 0);
	BuildStepIndex.Add(EBuildStep::OverlapRooms, 0);
	BuildStepIndex.Add(EBuildStep::OverlapCells, 0);

	if (!bUnitTest)
	{

		for (int32 i = 0; i < NumberOfCell; ++i)
		{
			FCellData NewCell;

			NewCell.Location = FVector::ZeroVector;

			NewCell.Size = FVector(FMath::RandRange((int32)MinSizeOfCell.X, (int32)MaxSizeOfCell.X),
				FMath::RandRange((int32)MinSizeOfCell.Y, (int32)MaxSizeOfCell.Y),
				0.f);

			NewCell.Size.Z = 1.f;

			NewCell.ID = i;
			AllCells.Add(NewCell);
		}

		Cells = &AllCells;
	}
	else
	{
		int32 idx = 0;
		for (auto& cell : UnitTestCells)
		{
			cell.ID = idx;
			idx++;
		}
		UnitTest();
		Cells = &UnitTestCells;
	}
}

void UGDMAlgorithmManager::CompleteCellSeparation()
{
	while (SeparateAllCell())
	{
		CompleteCellSeparation();
	}
}

bool UGDMAlgorithmManager::SeparateAllCell()
{
	bool bDidSeparate = false;

	for (auto& cell : *Cells)
	{
		bDidSeparate |= SeparateOneCell(cell);
	}

	return bDidSeparate;
}

bool UGDMAlgorithmManager::SeparateOneCellByIndex(int32 _Index)
{
	SeparateOneCell((*Cells)[_Index]);
	return true;
}

bool UGDMAlgorithmManager::SeparateOneCell(FCellData& _cell)
{
	FVector DirectionMove = FVector::ZeroVector;

	for (auto& cell : *Cells)
	{
		if (_cell != cell)
		{
			FVector CellToCell = _cell.Location - cell.Location;
			FVector CellToCellSize = (_cell.Size + cell.Size) / 2;

			float distance = (CellToCell).Length();

			bool bShouldMove = true;

			if (FMath::Abs<float>(CellToCell.X) >= FMath::Abs<float>(CellToCellSize.X)
				|| FMath::Abs<float>(CellToCell.Y) >= FMath::Abs<float>(CellToCellSize.Y)
				|| FMath::Abs<float>(CellToCell.Z) >= FMath::Abs<float>(CellToCellSize.Z))
				bShouldMove = false;

			if (bShouldMove)
			{
				int32 Rand = FMath::RandHelper(RandomChances.X + RandomChances.Y + RandomChances.Z);

				if (Rand < RandomChances.X)
					DirectionMove.X += CellToCell.X > 0 ? 0.5 : -0.5;
				else if (Rand < RandomChances.X + RandomChances.Y)
					DirectionMove.Y += CellToCell.Y > 0 ? 0.5 : -0.5;
				else
					DirectionMove.Z += CellToCell.Z > 0 ? 0.5 : -0.5;
			}

		}
	}

	_cell.Location += DirectionMove;

	if (DirectionMove == FVector::ZeroVector)
		return false;

	return true;
}

void UGDMAlgorithmManager::ProcessMainRooms()
{
	for (auto& cell : *Cells)
	{
		if (cell.Size.X >= ThresholdMainRooms.X && cell.Size.Y >= ThresholdMainRooms.Y)
		{
			MainRooms.Add(&cell);
		}
	}

}

void UGDMAlgorithmManager::ProcessDelaunay()
{
	for (auto aRoom : MainRooms)
	{
		for (auto bRoom : MainRooms)
		{
			if (aRoom != bRoom)
			{
				bool CreateEdge = true;

				FVector CircleCenter = bRoom->Location - aRoom->Location;
				float CircleRadius = CircleCenter.Length() / 2;

				CircleCenter = (CircleCenter / 2) + aRoom->Location;

				int32 NumInCirclePoints = 0;

				for (auto cRoom : MainRooms)
				{
					if (aRoom != cRoom && bRoom != cRoom)
					{
						if ((cRoom->Location - CircleCenter).Length() < CircleRadius)
							CreateEdge = false;
					}
				}
				
				if (CreateEdge)
					Edges.Add(FEdgeData(aRoom, bRoom));
			}
		}
	}
}

void UGDMAlgorithmManager::ProcessGraph()
{
	for (auto tri : Edges)
	{
		FGraphUnitData* NewGraphUnitA = Graph.FindByKey(*tri.CellA);
		FGraphUnitData* NewGraphUnitB = Graph.FindByKey(*tri.CellB);

		int32 IndexA = NewGraphUnitA == nullptr ? INDEX_NONE : Graph.Find(*NewGraphUnitA);
		int32 IndexB = NewGraphUnitB == nullptr ? INDEX_NONE : Graph.Find(*NewGraphUnitB);

		if (IndexA == INDEX_NONE)
			IndexA = Graph.Add(FGraphUnitData(tri.CellA, TArray<FCellData*>()));

		if (IndexB == INDEX_NONE)
			IndexB = Graph.Add(FGraphUnitData(tri.CellB, TArray<FCellData*>()));

		if (Graph[IndexA].Neighbors.Find(tri.CellB) == INDEX_NONE)
			Graph[IndexA].Neighbors.Add(tri.CellB);

		if (Graph[IndexB].Neighbors.Find(tri.CellA) == INDEX_NONE)
			Graph[IndexB].Neighbors.Add(tri.CellA);

	}
}

void UGDMAlgorithmManager::ProcessSimpleGraph()
{
	if (Graph.Num() > 0)
	{
		for (auto graph : Graph)
		{
			if (SimpleGraph.FindByKey(*graph.Cell) == nullptr)
			{
				int32 IndexCurrentUnit = SimpleGraph.Add(graph);
				BuildPathFromCell(IndexCurrentUnit, TArray<FCellData*>(&(SimpleGraph[IndexCurrentUnit].Cell),1));
			}
		}

		
	}
}

void UGDMAlgorithmManager::ProcessCompleteSimpleGraph()
{
	RandChancesNewEdge;

	for (int32 i = 0; i < SimpleGraph.Num(); ++i)
	{
		FGraphUnitData* CellInGraph = Graph.FindByKey(*SimpleGraph[i].Cell);

		for (auto neighb : CellInGraph->Neighbors)
		{
			if (SimpleGraph[i].Neighbors.FindByKey(neighb) == nullptr)
			{
				int32 Rand = FMath::RandHelper(RandChancesNewEdge);

				if (Rand == 1)
				{
					SimpleGraph[i].Neighbors.Add(neighb);
				}
			}
		}
	}

}

void UGDMAlgorithmManager::ProcessHallways()
{
	TArray<TPair<FCellData*,FCellData*>> CalcPair;

	for (auto sg : SimpleGraph)
	{
		for (auto neighb : sg.Neighbors)
		{
			if (CalcPair.FindByPredicate([sg, neighb](const TPair<FCellData*, FCellData*>& _Data) {
				return _Data.Key == sg.Cell && _Data.Value == neighb || _Data.Key == neighb && _Data.Value == sg.Cell; }) == nullptr)
			{
				FVector SizeSg = sg.Cell->Size / 2.f;
				SizeSg.Z = 0.f;
				FVector SizeNeighb = neighb->Size / 2.f;
				SizeNeighb.Z = 0.f;
				FVector SizeCells = SizeSg + SizeNeighb;
				FVector LocCells = sg.Cell->Location - neighb->Location;

				FVector CornerSg;
				FVector CornerNeighb;

				bool bCanCreateXHall = false;
				bool bCanCreateYHall = false;
				bool bCanCreateHall = true;

				FVector HallLoc = FVector::ZeroVector;
				FVector HallSize = FVector::ZeroVector;

				if (sg.Cell->Location.X >= neighb->Location.X)
				{
					if (sg.Cell->Location.Y >= neighb->Location.Y)
					{
						//DL
						CornerSg = sg.Cell->Location - SizeSg;
						//UR
						CornerNeighb = neighb->Location + SizeNeighb;

						if (CornerSg.X <= CornerNeighb.X && CornerSg.Y <= CornerNeighb.Y)
							bCanCreateHall = false;

						if (CornerSg.Y >= CornerNeighb.Y  && CornerSg.X <= CornerNeighb.X - HallwayThreshold)
							bCanCreateYHall = true;

						if (CornerSg.X >= CornerNeighb.X  && CornerSg.Y <= CornerNeighb.Y - HallwayThreshold)
							bCanCreateXHall = true;
					}

					if (sg.Cell->Location.Y < neighb->Location.Y)
					{
						//UL
						CornerSg = sg.Cell->Location;
						CornerSg.X -= SizeSg.X;
						CornerSg.Y += SizeSg.Y;
						//DR
						CornerNeighb = neighb->Location;
						CornerNeighb.X += SizeNeighb.X;
						CornerNeighb.Y -= SizeNeighb.Y;

						if (CornerSg.X <= CornerNeighb.X && CornerSg.Y >= CornerNeighb.Y)
							bCanCreateHall = false;

						if (CornerSg.Y < CornerNeighb.Y  && CornerSg.X <= CornerNeighb.X - HallwayThreshold)
							bCanCreateYHall = true;

						if (CornerSg.X >= CornerNeighb.X  && CornerSg.Y > CornerNeighb.Y + HallwayThreshold)
							bCanCreateXHall = true;
					}



				}

				if (sg.Cell->Location.X < neighb->Location.X)
				{
					if (sg.Cell->Location.Y <= neighb->Location.Y)
					{
						//UR
						CornerSg = sg.Cell->Location + SizeSg;
						//DL
						CornerNeighb = neighb->Location - SizeNeighb;

						if (CornerSg.X >= CornerNeighb.X && CornerSg.Y >= CornerNeighb.Y)
							bCanCreateHall = false;

						if (CornerSg.Y <= CornerNeighb.Y && CornerSg.X >= CornerNeighb.X + HallwayThreshold)
							bCanCreateYHall = true;

						if (CornerSg.X < CornerNeighb.X  && CornerSg.Y > CornerNeighb.Y + HallwayThreshold)
							bCanCreateXHall = true;
					}

					if (sg.Cell->Location.Y > neighb->Location.Y)
					{
						//DR
						CornerSg = sg.Cell->Location;
						CornerSg.X += SizeSg.X;
						CornerSg.Y -= SizeSg.Y;
						//UL
						CornerNeighb = neighb->Location;
						CornerNeighb.X -= SizeNeighb.X;
						CornerNeighb.Y += SizeNeighb.Y;

						if (CornerSg.X >= CornerNeighb.X && CornerSg.Y <= CornerNeighb.Y)
							bCanCreateHall = false;

						if (CornerSg.Y > CornerNeighb.Y  && CornerSg.X > CornerNeighb.X + HallwayThreshold)
							bCanCreateYHall = true;

						if (CornerSg.X < CornerNeighb.X  && CornerSg.Y < CornerNeighb.Y - HallwayThreshold)
							bCanCreateXHall = true;
					}
				}

				if (bCanCreateHall)
				{
					if (bCanCreateXHall)
					{
						HallLoc = CornerSg + ((CornerNeighb - CornerSg) / 2);
						HallSize = (CornerSg - CornerNeighb);
						HallSize.Y = 2.f;
						HallSize.X = FMath::Abs(HallSize.X);
						HallSize.Z = 1.f;

						AddHallway(FCellData(HallLoc, HallSize));
					}
					else if (bCanCreateYHall)
					{
						HallLoc = CornerSg + ((CornerNeighb - CornerSg) / 2);
						HallSize = (CornerSg - CornerNeighb);
						HallSize.X = 2.f;
						HallSize.Y = FMath::Abs(HallSize.Y);
						HallSize.Z = 1.f;

						AddHallway(FCellData(HallLoc, HallSize));
					}
					else
					{
						//HORIZ
						float CorSgToNeighbX = neighb->Location.X - CornerSg.X;
						CorSgToNeighbX += CorSgToNeighbX > 0.f ? 1.f : -1.f;
						HallLoc.X = CornerSg.X + (CorSgToNeighbX / 2);
						HallLoc.Y = sg.Cell->Location.Y;
						HallSize.Y = 2.f;
						HallSize.X = FMath::Abs(CorSgToNeighbX);
						HallSize.Z = 1.f;

						AddHallway(FCellData(HallLoc, HallSize));

						// VERT
						float CorNeighbToSgY = sg.Cell->Location.Y - CornerNeighb.Y;
						CorNeighbToSgY += CorNeighbToSgY > 0.f ? -1.f : 1.f;
						HallLoc.Y = CornerNeighb.Y + (CorNeighbToSgY / 2);
						HallLoc.X = neighb->Location.X;
						HallSize.X = 2.f;
						HallSize.Y = FMath::Abs(CorNeighbToSgY);
						HallSize.Z = 1.f;

						AddHallway(FCellData(HallLoc, HallSize));

						//HORIZ
						float CorNeighToSgX = CornerNeighb.X- sg.Cell->Location.X ;
						CorNeighToSgX += CorNeighToSgX > 0.f ? 1.f : -1.f;
						HallLoc.X = CornerNeighb.X - (CorNeighToSgX / 2);
						HallLoc.Y = neighb->Location.Y;
						HallSize.Y = 2.f;
						HallSize.X = FMath::Abs(CorNeighToSgX);
						HallSize.Z = 1.f;

						AddHallway(FCellData(HallLoc, HallSize));

						// VERT
						float NeigToCorSgY = neighb->Location.Y - CornerSg.Y ;
						NeigToCorSgY += NeigToCorSgY > 0.f ? -1.f : 1.f;
						HallLoc.Y = CornerSg.Y + (NeigToCorSgY / 2);
						HallLoc.X = sg.Cell->Location.X;
						HallSize.X = 2.f;
						HallSize.Y = FMath::Abs(NeigToCorSgY);
						HallSize.Z = 1.f;

						AddHallway(FCellData(HallLoc, HallSize));
					}
				}

				CalcPair.Add(TPair<FCellData*, FCellData*>(sg.Cell, neighb));
			}
		}
	}
}

TArray<FCellData> UGDMAlgorithmManager::ProcessOverlappingCellsByIndex(TArray<FCellData>& _ArrayToProcess, TArray<FCellData>& _ArrayToCompare, int32 _Idx)
{
	TArray<FCellData> ResultCells;
	TArray<FCellData> CellToDelete;

	bool bDeleteCell = false;

	for (int32 j = (_ArrayToCompare != _ArrayToProcess) ? 0 : _Idx + 1; j < _ArrayToCompare.Num(); ++j)
	{
		if (!CellToDelete.Contains(_ArrayToCompare[j]) && ((_ArrayToCompare != _ArrayToProcess) || (_ArrayToProcess[_Idx] != _ArrayToCompare[j])))
		{
			ECollisionType CollisionResult = AreCellCollisioned(&_ArrayToProcess[_Idx], &_ArrayToCompare[j]);
			if (CollisionResult == ECollisionType::Overlap || CollisionResult == ECollisionType::Inside)
			{
				bDeleteCell = true;
				/*if (CollisionResult == ECollisionType::Overlap)
					SplitedUnitCells.Append(ProcessSplitUnitCell(&_ArrayToProcess[_Idx], &_ArrayToCompare[j]));*/
			}
		}
	}

	if(!bDeleteCell)
		ResultCells.Add(_ArrayToProcess[_Idx]);

	return ResultCells;
}

void UGDMAlgorithmManager::BuildPathFromCell(int32 _CurrentIndex, TArray<FCellData*> _NewCells)
{
	int32 IndexCurrentUnit = _CurrentIndex;
	int32 IndexLastUnit = IndexCurrentUnit;

	bool ContinuePath = true;

	while (ContinuePath)
	{
		TArray<int32> RandIndex;

		for (int32 i = 0; i < SimpleGraph[IndexCurrentUnit].Neighbors.Num(); ++i)
		{
			if (_NewCells.Find(SimpleGraph[IndexCurrentUnit].Neighbors[i]) == INDEX_NONE)
			{
				RandIndex.Add(i);
			}
		}

		if (RandIndex.Num() > 0)
		{
			int32 KeepIndex = FMath::RandHelper(RandIndex.Num());

			FGraphUnitData* NeighborToKeep = Graph.FindByKey(*SimpleGraph[IndexCurrentUnit].Neighbors[RandIndex[KeepIndex]]);

			int32 IndexNeighToKeep = 0;

			if (nullptr != NeighborToKeep)
			{
				FGraphUnitData* NeighborToKeepSG = SimpleGraph.FindByKey(*NeighborToKeep->Cell);
				if (NeighborToKeepSG == nullptr)
				{
					IndexNeighToKeep = SimpleGraph.Add(FGraphUnitData(NeighborToKeep->Cell, NeighborToKeep->Neighbors));
					_NewCells.Add(NeighborToKeep->Cell);
				}
				else
				{
					IndexNeighToKeep = SimpleGraph.Find(*NeighborToKeepSG);
					SimpleGraph[IndexNeighToKeep].Neighbors.Add(SimpleGraph[IndexCurrentUnit].Cell);
					ContinuePath = false;
				}
				
			}
			else
				IndexNeighToKeep = SimpleGraph.Add(FGraphUnitData(SimpleGraph[IndexCurrentUnit].Neighbors[RandIndex[KeepIndex]], TArray<FCellData*>()));

			SimpleGraph[IndexCurrentUnit].Neighbors.Empty();
			SimpleGraph[IndexCurrentUnit].Neighbors.Add(SimpleGraph[IndexNeighToKeep].Cell);

			if (IndexCurrentUnit != IndexLastUnit)
				SimpleGraph[IndexCurrentUnit].Neighbors.Add(SimpleGraph[IndexLastUnit].Cell);

			IndexLastUnit = IndexCurrentUnit;
			IndexCurrentUnit = IndexNeighToKeep;
		}
		else
		{
			SimpleGraph[IndexCurrentUnit].Neighbors.Empty();
			SimpleGraph[IndexCurrentUnit].Neighbors.Add(SimpleGraph[IndexLastUnit].Cell);
			ContinuePath = false;
		}
	}
}

void UGDMAlgorithmManager::Reset()
{
	Cells = nullptr;
	AllCells.Empty();

	MainRooms.Empty();
	HallRooms.Empty();

	Triangles.Empty();
	Edges.Empty();

	Graph.Empty();
	SimpleGraph.Empty();

	Hallways.Empty();
	ProcessedHallways.Empty();

	UnitCells.Empty();
	ProcessedUnitCells.Empty();
	SplitedUnitCells.Empty();
	HallwaysToUnitSplit.Empty();

	OverlappingHallways.Empty();

	CurrentBuildStep = EBuildStep::SeparateCells;
}

ECollisionType UGDMAlgorithmManager::AreCellCollisioned(FCellData* _Cell1, FCellData* _Cell2)
{
	FVector Cell1Size = _Cell1->Size / 2;
	FVector Cell2Size = _Cell2->Size / 2;

	float Cell1WithSizeX = Cell1Size.X + _Cell1->Location.X + Cell2Size.X;
	float Cell1WithSizeY = Cell1Size.Y + _Cell1->Location.Y + Cell2Size.Y;

	float Cell1WithoutSizeX = _Cell1->Location.X - Cell1Size.X - Cell2Size.X;
	float Cell1WithoutSizeY = _Cell1->Location.Y - Cell1Size.Y - Cell2Size.Y;

	if ((_Cell2->Location.X < Cell1WithSizeX + CellCollisionThreshold && _Cell2->Location.X > Cell1WithSizeX - CellCollisionThreshold)
		|| (_Cell2->Location.X < Cell1WithoutSizeX + CellCollisionThreshold && _Cell2->Location.X > Cell1WithoutSizeX - CellCollisionThreshold)
		|| (_Cell2->Location.Y < Cell1WithSizeY + CellCollisionThreshold && _Cell2->Location.Y > Cell1WithSizeY - CellCollisionThreshold)
		|| (_Cell2->Location.Y < Cell1WithoutSizeY + CellCollisionThreshold && _Cell2->Location.Y > Cell1WithoutSizeY - CellCollisionThreshold))
		return ECollisionType::Touching;

	if (_Cell2->Location.X + Cell2Size.X >= _Cell1->Location.X + Cell1Size.X
		&& _Cell2->Location.X - Cell2Size.X <= _Cell1->Location.X - Cell1Size.X
		&& _Cell2->Location.Y + Cell2Size.Y >= _Cell1->Location.Y + Cell1Size.Y
		&& _Cell2->Location.Y - Cell2Size.Y <= _Cell1->Location.Y - Cell1Size.Y)
		return ECollisionType::Inside;

	if (_Cell2->Location.X > Cell1WithSizeX
		|| _Cell2->Location.X < Cell1WithoutSizeX 
		|| _Cell2->Location.Y > Cell1WithSizeY
		|| _Cell2->Location.Y < Cell1WithoutSizeY)
	return ECollisionType::Collision_None;
	
	return ECollisionType::Overlap;
}

// Creates hole of halfsize in best case and full holes in worst cases (because of floating approximation)
TArray<FCellData> UGDMAlgorithmManager::ProcessUnitCellFromCell(FCellData& _Cell, float _Unit, int32 _ID)
{
	TArray<FCellData> ResultUnitCells;

	for (float i = -(_Cell.Size.X/2); i < _Cell.Size.X / 2; i += _Unit)
	{
		for (float j = -(_Cell.Size.Y / 2); j < _Cell.Size.Y /2; j += _Unit)
		{
			ResultUnitCells.Add(FCellData(FVector(i+ (_Unit/2), j + (_Unit/2), 0.f) + _Cell.Location, FVector(_Unit, _Unit, 1.f), _ID));
			_ID++;
		}
	}

	return ResultUnitCells;
}

TArray<FCellData> UGDMAlgorithmManager::ProcessSplitUnitCell(FCellData* _CellKeep, FCellData* _CellSplit)
{
	bool SplitLeft = false;
	bool SplitRight = false;
	bool SplitUp = false;
	bool SplitDown = false;

	FVector HalfSizeKeep = _CellKeep->Size / 2;
	FVector HalfSizeSplit = _CellSplit->Size / 2;

	TArray<FCellData> ResultHalfUnitCell;

	if ((_CellSplit->Location.X - HalfSizeSplit.X) < (_CellKeep->Location.X - HalfSizeKeep.X))
	{
		SplitLeft = true;
	}

	if ((_CellSplit->Location.X + HalfSizeSplit.X) > (_CellKeep->Location.X + HalfSizeKeep.X))
	{
		SplitRight = true;
	}

	if ((_CellSplit->Location.Y - HalfSizeSplit.Y) < (_CellKeep->Location.Y - HalfSizeKeep.Y))
	{
		SplitDown = true;
	}

	if ((_CellSplit->Location.Y + HalfSizeSplit.Y) > (_CellKeep->Location.Y + HalfSizeKeep.Y))
	{
		SplitUp = true;
	}

	TArray<FCellData> SplitCell = ProcessUnitCellFromCell(*_CellSplit, 0.25f,0);
	TArray<int32> IndexKeep;

	if (SplitLeft)
	{
		IndexKeep.AddUnique(0);
		IndexKeep.AddUnique(1);
	}

	if (SplitRight)
	{
		IndexKeep.AddUnique(2);
		IndexKeep.AddUnique(3);
	}

	if (SplitDown)
	{
		IndexKeep.AddUnique(0);
		IndexKeep.AddUnique(2);
	}

	if (SplitUp)
	{
		IndexKeep.AddUnique(1);
		IndexKeep.AddUnique(3);
	}

	for (auto idx : IndexKeep)
	{
		ResultHalfUnitCell.Add(SplitCell[idx]);
	}

	return ResultHalfUnitCell;
}

void UGDMAlgorithmManager::AddHallway(FCellData _NewCell)
{
	_NewCell.ID = Hallways.Num();
	Hallways.Add(_NewCell);
}

void UGDMAlgorithmManager::UnitTest()
{
	for (auto& cell : UnitTestCells)
	{
		for (auto& cell1 : UnitTestCells)
		{
			ECollisionType Result = AreCellCollisioned(&cell, &cell1);
			if(Result == ECollisionType::Inside)
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("coucou"));
		}
	}
}

EBuildStep UGDMAlgorithmManager::BuildStep()
{
	switch (CurrentBuildStep)
	{
		//Separate all cells from each other
	case EBuildStep::SeparateCells:
		BuildStepSeparateCells();
		break;

		//Process main rooms and the graphs for connecting the rooms
	case EBuildStep::Graph:
		BuildStepGraph();
		break;

		// Process hallways that needs to into unit cells, build clusters to compare the collision with the minimum of cells
	case EBuildStep::UnitCells:
		BuildStepUnitCells();
		break;

		// Check unit cells collision with the mains/small rooms
	case EBuildStep::OverlapRooms:
		BuildStepOverlapRooms();
		break;
		
		// Compare the unit cells with each other from the collided hallways
	case EBuildStep::OverlapCells:
		BuildStepOverlapCells();
		break;
	}

	return CurrentBuildStep;
}

void UGDMAlgorithmManager::BuildStepSeparateCells()
{
	if (!SeparateAllCell())
		CurrentBuildStep = EBuildStep::Graph;
}

void UGDMAlgorithmManager::BuildStepGraph()
{
	//Select the rooms
	ProcessMainRooms();
	//Process delaunay graph (not the real delaunay algo)
	ProcessDelaunay();
	// Process graph with unique edges
	ProcessGraph();
	//Process simplegraph without loop
	ProcessSimpleGraph();
	// Process complete graph with adding random connection
	ProcessCompleteSimpleGraph();
	// Process hallways between main rooms
	ProcessHallways();
	// Check Collision between hallways 
	CheckHallwaysToSplitIntoUnit();

	CurrentBuildStep = EBuildStep::UnitCells;
}

void UGDMAlgorithmManager::BuildStepUnitCells()
{
	if (OverlappingHallways.Num() > BuildStepIndex[EBuildStep::UnitCells])
	{
		OverlappingHallways[BuildStepIndex[EBuildStep::UnitCells]].UnitCells.Append(ProcessUnitCellFromCell(OverlappingHallways[BuildStepIndex[EBuildStep::UnitCells]].Original, 0.5f, OverlappingHallways[BuildStepIndex[EBuildStep::UnitCells]].UnitCells.Num()));
		BuildStepIndex[EBuildStep::UnitCells]++;
	}
	else
	{
		CurrentBuildStep = EBuildStep::OverlapRooms;
		BuildStepIndex[EBuildStep::UnitCells] = 0;
	}
}

void UGDMAlgorithmManager::BuildStepOverlapRooms()
{
	if (OverlappingHallways.Num() > BuildStepIndex[EBuildStep::OverlapRooms])
	{
		int32 Idx = 0;
		for (auto& cell : OverlappingHallways[BuildStepIndex[OverlapRooms]].UnitCells)
		{
			OverlappingHallways[BuildStepIndex[OverlapRooms]].ProcessedUnitCells.Append(ProcessOverlappingCellsByIndex(OverlappingHallways[BuildStepIndex[OverlapRooms]].UnitCells, *Cells, Idx));
			Idx++;
		}
		BuildStepIndex[OverlapRooms]++;
	}
	else
	{
		CurrentBuildStep = EBuildStep::OverlapCells;
		BuildStepIndex[EBuildStep::OverlapRooms] = 0;
	}
}

void UGDMAlgorithmManager::BuildStepOverlapCells()
{
	if (OverlappingHallways.Num() > BuildStepIndex[EBuildStep::OverlapCells])
	{

		for (auto& cell : OverlappingHallways[BuildStepIndex[OverlapCells]].OverlapCells)
		{
			FCellOverlapData* Ref = OverlappingHallways.FindByKey(cell);
			if (nullptr != Ref)
			{
				for (int32 i = 0; i < OverlappingHallways[BuildStepIndex[OverlapCells]].ProcessedUnitCells.Num(); ++i)
					OverlappingHallways[BuildStepIndex[OverlapCells]].ProcessedHallways.Append(ProcessOverlappingCellsByIndex(OverlappingHallways[BuildStepIndex[OverlapCells]].ProcessedUnitCells, Ref->ProcessedHallways.Num() > 0 ? Ref->ProcessedUnitCells : ProcessedHallways, i));
			}
		}
		//If we don't collide with another hallway we juste add the hallway to the processed
		if (OverlappingHallways[BuildStepIndex[OverlapCells]].OverlapCells.Num() == 0)
			OverlappingHallways[BuildStepIndex[OverlapCells]].ProcessedHallways = OverlappingHallways[BuildStepIndex[OverlapCells]].ProcessedUnitCells;
		BuildStepIndex[OverlapCells]++;
	}
	else
	{
		CurrentBuildStep = EBuildStep::Finished;
		BuildStepIndex[EBuildStep::OverlapCells] = 0;
	}
}

void UGDMAlgorithmManager::CheckHallwaysToSplitIntoUnit()
{
	bool bAdd = false;

	TArray<int32> IndexNotCollisioned;

	for (int32 i = 0; i < Hallways.Num(); ++i)
	{
		bAdd = false;
		for (int32 j = 0; j < Hallways.Num(); ++j)
		{
			ECollisionType CollisionResult = AreCellCollisioned(&Hallways[i], &Hallways[j]);
			if (CollisionResult == ECollisionType::Overlap && i != j )
			{
				FCellData CurrentHall = Hallways[i];
				FCellOverlapData* NewOverlap = OverlappingHallways.FindByPredicate([CurrentHall](const FCellOverlapData& _Data) {
					return _Data.Original == CurrentHall; });

				if (NewOverlap == nullptr)
				{
					FCellOverlapData TempOverlap;
					TempOverlap.Original = Hallways[i];
					TempOverlap.OverlapCells.Add(Hallways[j]);
					OverlappingHallways.Add(TempOverlap);
				}
				else
					NewOverlap->OverlapCells.Add(Hallways[j]);

				bAdd = true;
			}
		}
		if (!bAdd)
			IndexNotCollisioned.Add(i);
	}

	for (int32 i = 0; i < Hallways.Num(); ++i)
	{
		bAdd = false;
		if (!HallwaysToUnitSplit.Contains(Hallways[i]))
		{
			for (int32 j = 0; j < Cells->Num(); ++j)
			{
				ECollisionType CollisionResult = AreCellCollisioned(&Hallways[i], &(*Cells)[j]);
				if (CollisionResult == ECollisionType::Overlap)
				{
					if (Cells->Contains((*Cells)[j]) && !MainRooms.Contains(&(*Cells)[j]))
					{
						HallRooms.AddUnique(&(*Cells)[j]);
					}
					FCellData CurrentHall = Hallways[i];
					FCellOverlapData* NewOverlap = OverlappingHallways.FindByPredicate([CurrentHall](const FCellOverlapData& _Data) {
						return _Data.Original == CurrentHall; });

					if (NewOverlap == nullptr)
					{
						FCellOverlapData TempOverlap;
						TempOverlap.Original = Hallways[i];
						OverlappingHallways.Add(TempOverlap);
					}

					IndexNotCollisioned.Remove(i);
						bAdd = true;
				}
			}
		}
	}

	for (auto Idx : IndexNotCollisioned)
	{
		ProcessedHallways.Add(Hallways[Idx]);
	}
}