#include "GDMDungeonRoom.h"
#include "Components/BoxComponent.h"

AGDMDungeonRoom::AGDMDungeonRoom(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = BoxComponent;

	FloorComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorComp"));
	FloorComponent->SetupAttachment(RootComponent);
	PrimaryActorTick.bCanEverTick = true;
}
#pragma optimize("",off)
void AGDMDungeonRoom::BeginPlay()
{
	Super::BeginPlay();

	if (nullptr != BoxComponent)
	{
		BoxComponent->SetHiddenInGame(false);
		BoxComponent->SetVisibility(true);
	}
}

void AGDMDungeonRoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGDMDungeonRoom::SetupRoomTransform(FTransform& _transform, FColor _ShapeColor, float _MeasureUnit, bool _draw)
{
	BoxComponent->SetBoxExtent(_transform.GetScale3D() / 2, true);
	FloorComponent->SetRelativeScale3D(_transform.GetScale3D() / _MeasureUnit);
	BoxComponent->ShapeColor = _ShapeColor;
	_transform.SetScale3D(FVector(1.f,1.f,1.f));
	SetActorTransform(_transform);
	SetActorHiddenInGame(!_draw);
}

#pragma optimize("",on)