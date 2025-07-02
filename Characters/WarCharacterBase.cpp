#include "WarCharacterBase.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "DataManager/ConfigData/AbilitiesConfig.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "WarComponents/PersistentSystem/WarPersistentSystem.h"
#include "War/WarComponents/AbilitySystem/WarAbilitySystemComponent.h"


AWarCharacterBase::AWarCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	AbilitiesConfig = CreateDefaultSubobject<UAbilitiesConfig>(TEXT("AbilitiesConfig"));
	if (!AbilitiesConfig)
	{
		print_err(TEXT("Failed to create AbilitiesConfig"));
	}
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

void AWarCharacterBase::LoadActorData(FMemoryReader& MemoryReader)
{
}


//人物ID需要固定，从配置文件拿
void AWarCharacterBase::SetPersistentID(const FGuid& NewID)
{
	PersistentID = FGuid(UWarGameInstanceSubSystem::GetStaticPlayerID(this));
}

UAbilitySystemComponent* AWarCharacterBase::GetAbilitySystemComponent() const
{
	return WarAbilitySystemComponent;
}

UWarAttributeSet* AWarCharacterBase::GetAttributeSet() const
{
	return WarAttributeSet;
}


void AWarCharacterBase::InitDefaultAttributes() const
{
	if (!WarAbilitySystemComponent || !DefaultAttributeEffect) return;

	FGameplayEffectContextHandle EffectContext = WarAbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = WarAbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect, 1.f, EffectContext);

	if (SpecHandle.IsValid())
	{
		WarAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
