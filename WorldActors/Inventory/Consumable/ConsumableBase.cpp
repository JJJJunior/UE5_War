

#include "ConsumableBase.h"


AConsumableBase::AConsumableBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AConsumableBase::BeginPlay()
{
	Super::BeginPlay();
}
