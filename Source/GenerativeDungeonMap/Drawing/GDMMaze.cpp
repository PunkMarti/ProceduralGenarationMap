#include "GDMMaze.h"
#include "GDMAlgorithmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/TextRenderComponent.h"

AGDMMaze::AGDMMaze(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MazeAlgorithmComponent = CreateDefaultSubobject<UGDMAlgorithmComponent>("AlgorithmComp");
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	FloorMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
	WallMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall"));
	StartText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Start"));
	TreasureText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Treasure"));
	ExitText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Exit"));
	
	FloorMeshComponent->SetupAttachment(RootComponent);
	WallMeshComponent->SetupAttachment(RootComponent);
	StartText->SetupAttachment(RootComponent);
	TreasureText->SetupAttachment(RootComponent);
	ExitText->SetupAttachment(RootComponent);

	PrimaryActorTick.bCanEverTick = true;
}

void AGDMMaze::BeginPlay()
{
	Super::BeginPlay();

	InitMazeBody();
}

void AGDMMaze::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (bBuildOverTime)
	{
		// If we want to see the maze being built over time we call one building step at a time (every tick)
		GenerateDynamicMaze();

		// We stop the processing when the maze is fully built
		if (nullptr != MazeAlgorithmComponent && MazeAlgorithmComponent->IsGenerationOver())
		{
			bBuildOverTime = false;
			MazeAlgorithmComponent->GenerateRooms();
			CreateRooms();
		}
	}
}

void AGDMMaze::InitMazeBody()
{
	if (nullptr != MazeAlgorithmComponent)
	{
		// Hide the original wall used for sampling walls
		WallMeshComponent->SetVisibility(false);

		MazeAlgorithmComponent->InitMaze();

		//Set the floor
		FloorMeshComponent->SetRelativeScale3D(FVector(GetMazeRealWidth() / RoomUnitSize, GetMazeRealHeight() / RoomUnitSize, FloorMeshComponent->GetRelativeScale3D().Z));

		// Process the whole maze 
		if (!bBuildOverTime)
		{
			MazeAlgorithmComponent->GenerateMaze();

		}
		GenerateMazeBody();
	}
}

void AGDMMaze::GenerateMazeBody()
{
	// HorizontalWalls placed
	for (int32 i = 0; i < MazeAlgorithmComponent->GetNumberOfHorizontalHeightWall(); i += 1)
	{
		int32 CurrentHIndex = i * 2;

		for (int32 j = 0; j < MazeAlgorithmComponent->GetNumberOfHorizontalWidthWall(); ++j)
			CreateHorizontalWall(j, CurrentHIndex);
	}

	// VerticalWalls placed
	for (int32 i = 0; i < MazeAlgorithmComponent->GetNumberOfVerticalHeightWall(); i += 1)
	{
		int32 CurrentHIndex = (i * 2) + 1;

		for (int32 j = 0; j < MazeAlgorithmComponent->GetNumberOfVerticalWidthWall(); ++j)
			CreateVerticalWall(j, CurrentHIndex);

	}

	//Rooms
	CreateRooms();
}

void AGDMMaze::GenerateDynamicMaze()
{
	int32 xH = -1;
	int32 yH = -1;
	int32 xV = -1;
	int32 yV = -1;

	// Try to build a wall and if we succeed create it physically
	if (nullptr != MazeAlgorithmComponent && MazeAlgorithmComponent->PlaceWall(xH, yH, xV, yV))
	{
		if (xH != -1 && yH != -1)
			CreateHorizontalWall(xH, yH);

		if (xV != -1 && yV != -1)
			CreateVerticalWall(xV, yV);
	}
	// If we didn't manage to build a wall, recursive call until we manage or we reach the limit of tries
	else if (nullptr != MazeAlgorithmComponent && !MazeAlgorithmComponent->IsGenerationOver())
		GenerateDynamicMaze();
}

FVector AGDMMaze::GetWallLocationFromCoord(int32 _x, int32 _y)
{
	float RoomRealSize = GetRoomSize();

	// Compute the real location of the wall based on the measurments set
	// Offset applied to the x for the vertical wall of half the room size

	float PosY = (_y * (RoomRealSize/2));

	float PosX = (_x * RoomRealSize) + ((_y % 2) ==1 ? 0 : (RoomRealSize / 2));

	PosY -= GetMazeRealHeight() / 2;

	PosX -= GetMazeRealWidth() / 2;

	return FVector(PosX, PosY, 0.f);
}

FVector AGDMMaze::GetRoomLocationFromIndex(int32 _Index)
{
	// Compute the real location of the room based on the measurements set

	float PosY = ((_Index / MazeAlgorithmComponent->Width) * GetRoomSize()) + (GetRoomSize()/ 2);

	float PosX = ((_Index % MazeAlgorithmComponent->Width) * GetRoomSize()) + (GetRoomSize() / 2);

	PosY -= GetMazeRealHeight() / 2;

	PosX -= GetMazeRealWidth() / 2;

	return FVector(PosX, PosY, 0.f);
}

float AGDMMaze::GetMazeRealWidth()
{
	// The maze real width is the number of vertical wall on a line 
	return MazeAlgorithmComponent->GetNumberOfHorizontalWidthWall() * GetRoomSize();
}

float AGDMMaze::GetMazeRealHeight()
{
	// The maze real height is the number of horizontal wall on a column 
	return MazeAlgorithmComponent->GetNumberOfVerticalHeightWall() * GetRoomSize();
}

float AGDMMaze::GetRoomSize()
{
	// Measurements
	return RoomScaleValue * RoomUnitSize;
}

void AGDMMaze::CreateNewWall(FTransform _WallTransform)
{
	// Create the body of the wall

	if (nullptr != WallMeshComponent)
	{
		UStaticMeshComponent* NewComp = Cast<UStaticMeshComponent>(AddComponentByClass(WallMeshComponent->GetClass(), false, _WallTransform, false));

		if (nullptr != NewComp)
		{
			NewComp->SetStaticMesh(WallMeshComponent->GetStaticMesh());
			NewComp->SetMaterial(0, WallMeshComponent->GetMaterial(0));
			NewComp->RegisterComponent();

		}
	}
}

void AGDMMaze::CreateHorizontalWall(int32 _i, int32 _j)
{
	// Compute the Transform of the horizontal wall

	FQuat HorizontalWallQuat = UKismetMathLibrary::MakeRotFromX(FVector(1.f, 0.f, 0.f)).Quaternion();
	uint8 Wall = MazeAlgorithmComponent->GetWall(_i, _j);

	if (Wall == 1)
	{
		FTransform WallTransform;
		WallTransform.SetScale3D(WallMeshComponent->GetRelativeScale3D());
		WallTransform.SetLocation(GetWallLocationFromCoord(_i, _j));
		WallTransform.SetRotation(HorizontalWallQuat);

		CreateNewWall(WallTransform);
	}
}

void AGDMMaze::CreateVerticalWall(int32 _i, int32 _j)
{
	// Compute the Transform of the vertical wall

	FQuat VerticalWallQuat = UKismetMathLibrary::MakeRotFromX(FVector(0.f, 1.f, 0.f)).Quaternion();

	uint8 Wall = MazeAlgorithmComponent->GetWall(_i, _j);

	if (Wall == 1)
	{
		FTransform WallTransform;
		WallTransform.SetScale3D(WallMeshComponent->GetRelativeScale3D());
		WallTransform.SetLocation(GetWallLocationFromCoord(_i, _j));
		WallTransform.SetRotation(VerticalWallQuat);

		CreateNewWall(WallTransform);
	}
}

void AGDMMaze::CreateRooms()
{
	// Compute the Transform of the rooms

	StartText->SetRelativeLocation(GetRoomLocationFromIndex(MazeAlgorithmComponent->StartIndex) + OffSetText);
	TreasureText->SetRelativeLocation(GetRoomLocationFromIndex(MazeAlgorithmComponent->TreasureIndex) + OffSetText);
	ExitText->SetRelativeLocation(GetRoomLocationFromIndex(MazeAlgorithmComponent->ExitIndex) + OffSetText);
}

void AGDMMaze::ResetMaze(bool _bInstantProcess, int32 _Width, int32 _Height)
{
	bBuildOverTime = !_bInstantProcess;
	_Width = FMath::Clamp(_Width, 2, 255);
	_Height = FMath::Clamp(_Height, 2, 255);
	MazeAlgorithmComponent->ResetMaze(_Width, _Height);

	TArray<UActorComponent*> AllMesh = GetComponentsByClass(UStaticMeshComponent::StaticClass());
	AllMesh.Remove(WallMeshComponent);
	AllMesh.Remove(FloorMeshComponent);
	for (auto mesh : AllMesh)
		mesh->DestroyComponent();
	AllMesh.Empty();
	InitMazeBody();
}
