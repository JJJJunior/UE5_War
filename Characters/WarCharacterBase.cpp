#include "WarCharacterBase.h"


AWarCharacterBase::AWarCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWarCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}
