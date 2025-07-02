#include "WarPlayerState.h"
#include "War/WarComponents/AbilitySystem/WarAbilitySystemComponent.h"
#include "WarComponents/AbilitySystem/WarAttributeSet.h"

AWarPlayerState::AWarPlayerState()
{
	SetNetUpdateFrequency(100.f);

	WarAbilitySystemComponent = CreateDefaultSubobject<UWarAbilitySystemComponent>("WarAbilitySystemComponent");
	WarAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	WarAttributeSet = CreateDefaultSubobject<UWarAttributeSet>("WarAttributeSet");
}

UAbilitySystemComponent* AWarPlayerState::GetAbilitySystemComponent() const
{
	return WarAbilitySystemComponent;
}

UWarAttributeSet* AWarPlayerState::GetAttributeSet() const
{
	return WarAttributeSet;
}
