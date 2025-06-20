#include "WarCharacterBase.h"
#include "WarComponents/PersistentSystem/WarPersistentSystem.h"

AWarCharacterBase::AWarCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWarCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWarCharacterBase::SaveActorData(FMemoryWriter& MemoryWriter) const
{
	// 如果有其他数据（装备、技能CD等），继续写
	// int32 HealthToSave = CurrentHealth;
	// MemoryWriter << HealthToSave;
}

void AWarCharacterBase::LoadActorData(FMemoryReader& MemoryReader) const
{
	// 如果有其他数据（装备、技能CD等），继续读
	//  MemoryReader << LoadedHealth;
	//  CurrentHealth = LoadedHealth;
}

void AWarCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	PersistentActorID = UWarPersistentSystem::SetPersistentActorGuid();
}
