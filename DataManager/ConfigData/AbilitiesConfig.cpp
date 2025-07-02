#include "AbilitiesConfig.h"
#include "WarComponents/AbilitySystem/WarAbilitySystemComponent.h"

void UAbilitiesConfig::GiveAbilities(const TObjectPtr<UWarAbilitySystemComponent>& ASC) const
{
	if (DefaultAbilities.Num() > 0)
	{
		for (const auto& Item : DefaultAbilities)
		{
			const FGameplayAbilitySpec AbilitySpec(Item.Value, 1,static_cast<int32>(Item.Key));
			ASC->GiveAbility(AbilitySpec);
		}
	}
}
