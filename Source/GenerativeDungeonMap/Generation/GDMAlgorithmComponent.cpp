#include "GDMAlgorithmComponent.h"

void UGDMAlgorithmComponent::InitMaze()
{
	Walls.Init(0, (GetNumberOfHorizontalHeightWall() * GetNumberOfHorizontalWidthWall()) + (GetNumberOfVerticalHeightWall() * GetNumberOfVerticalWidthWall()));

	int32 CurrentIndex = 0;

	CurrentFailAttemptNumber = 0;

	// Placing all the horizontal walls for the outside walls of the maze
	for (int32 i = 0; i < GetNumberOfHorizontalHeightWall(); i += GetNumberOfHorizontalHeightWall() -1)
	{
		int32 CurrentHIndex = i * 2;
		for (int32 j = 0; j < GetNumberOfHorizontalWidthWall(); ++j)
		{
			CurrentIndex = GetIndexWall(j, CurrentHIndex);

			if (Walls.Num() > CurrentIndex)
				Walls[CurrentIndex] = 1;
		}
	}

	// Placing all the Vertical walls for the outside walls of the maze
	for (int32 i = 0; i < GetNumberOfVerticalHeightWall(); i += 1)
	{
		int32 CurrentHIndex = (i * 2) + 1;
		for (int32 j = 0; j < GetNumberOfVerticalWidthWall(); j += GetNumberOfVerticalWidthWall() - 1)
		{
			CurrentIndex = GetIndexWall(j, CurrentHIndex);

			if (Walls.Num() > CurrentIndex)
				Walls[CurrentIndex] = 1;
		}
	}
}

void UGDMAlgorithmComponent::GenerateMaze()
{
	do
	{
		int32 xH;
		int32 yH;
		int32 xV;
		int32 yV;
		PlaceWall(xH, yH, xV, yV);

	} while (!IsGenerationOver());

	GenerateRooms();
}

bool UGDMAlgorithmComponent::PlaceWall(int32& _xH, int32& _yH, int32& _xV, int32& _yV)
{
	bool bWallPlaced = false;
	
	//
	// The algorithm consist of placing a wall if it's connected only to one of it's end to another wall so we never close the paths
	//

	// VERTICAL WALL

	int32 i = FMath::RandHelper(GetNumberOfVerticalWidthWall());
	int32 j = (FMath::RandHelper(GetNumberOfVerticalHeightWall()) * 2) + 1;

	int32 CurrentIndex = GetIndexWall(i, j);

	if (Walls[CurrentIndex] != 1)
	{
		bool Up = false;
		bool Down = false;

		// Test all the walls around to see if we connect to one 

		// Test the vertical wall above
		if (j > 1)
		{
			Up = Walls[CurrentIndex - GetNumberOfHorizontalWidthWall() - GetNumberOfVerticalWidthWall()] && 1;
		}

		// Test the vertical wall below
		if (j < ((GetNumberOfVerticalHeightWall() * 2) - 1))
		{
			Down = Walls[CurrentIndex + GetNumberOfHorizontalWidthWall() + GetNumberOfVerticalWidthWall()] && 1;
		}

		// Test the horizontal walls on the left
		if (i > 0)
		{
			int32 NUL = CurrentIndex - GetNumberOfHorizontalWidthWall() - 1;
			int32 NDL = CurrentIndex + GetNumberOfHorizontalWidthWall();
			Up = Up || Walls[NUL];
			Down = Down || Walls[NDL];
		}

		// Test the horizontal walls on the right
		if (i < (GetNumberOfVerticalWidthWall() - 1))
		{
			int32 NUR = CurrentIndex - GetNumberOfHorizontalWidthWall();
			int32 NDR = CurrentIndex + GetNumberOfHorizontalWidthWall() + 1;
			Up = Up || Walls[NUR];
			Down = Down || Walls[NDR];
		}

		if ((Up || Down) && !(Up && Down))
		{
			Walls[CurrentIndex] = 1;
			bWallPlaced = true;

			_xV = i;
			_yV = j;
		}
	}

	// HORIZONTAL WALL

	i = FMath::RandHelper(GetNumberOfHorizontalWidthWall());
	j = (FMath::RandHelper(GetNumberOfHorizontalHeightWall()) * 2);

	CurrentIndex = GetIndexWall(i, j);

	if (Walls[CurrentIndex] != 1)
	{
		bool Left = false;
		bool Right = false;

		// Test the horizontal wall on the left
		if (i > 0)
		{
			Left = Walls[CurrentIndex - 1] && 1;
		}

		// Test the horizontal wall on the right
		if (i < (GetNumberOfHorizontalWidthWall() - 1))
		{
			Right = Walls[CurrentIndex + 1] && 1;
		}

		// Test the vertcial walls below
		if (j < ((GetNumberOfHorizontalHeightWall() - 1) * 2))
		{
			int32 NLD = CurrentIndex + GetNumberOfHorizontalWidthWall();
			int32 NRD = CurrentIndex + GetNumberOfHorizontalWidthWall() + 1;
			Left = Left || Walls[NLD];
			Right = Right || Walls[NRD];
		}

		// Test the vertcial walls above
		if (j > 0)
		{
			int32 NLU = CurrentIndex - GetNumberOfHorizontalWidthWall() - 1;
			int32 NRU = CurrentIndex - GetNumberOfHorizontalWidthWall();
			Left = Left || Walls[NLU];
			Right = Right || Walls[NRU];
		}

		if ((Left || Right) && !(Left && Right))
		{
			Walls[CurrentIndex] = 1;
			bWallPlaced = true;

			_xH = i;
			_yH = j;
		}
	}


	if (!bWallPlaced)
		CurrentFailAttemptNumber++;
	else
		CurrentFailAttemptNumber = 0;

	return bWallPlaced;
}

void UGDMAlgorithmComponent::GenerateRooms()
{
	// Generate rooms only if they have different index and the size of the maze allow it
	do
	{
		StartIndex = FMath::RandHelper(Width * Height);
		TreasureIndex = FMath::RandHelper(Width * Height);
		ExitIndex = FMath::RandHelper(Width * Height);
	} while ((StartIndex == TreasureIndex || ExitIndex == TreasureIndex || StartIndex == ExitIndex) && (Width * Height) > 3);
}

void UGDMAlgorithmComponent::ResetMaze(int32 _Width, int32 _Height)
{
	Walls.Empty();
	InitMaze();
	Width = _Width > 255 ? 255 : _Width;
	Height = _Height > 255 ? 255 : _Height;
}