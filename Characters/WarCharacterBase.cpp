#include "WarCharacterBase.h"

#include "GameInstance/WarGameInstanceSubSystem.h"
#include "Kismet/GameplayStatics.h"
#include "WarComponents/PersistentSystem/WarPersistentSystem.h"


AWarCharacterBase::AWarCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWarCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	//分配全局ID
	UWarPersistentSystem::GeneratorPersistentID(this);
}

void AWarCharacterBase::SaveActorData(FMemoryWriter& MemoryWriter) const
{
}

void AWarCharacterBase::LoadActorData(FMemoryReader& MemoryReader) const
{
}


//人物ID需要固定，从配置文件拿
void AWarCharacterBase::SetPersistentID(const FGuid& NewID)
{
	PersistentID = FGuid(UWarGameInstanceSubSystem::GetStaticPlayerID(this));
}
