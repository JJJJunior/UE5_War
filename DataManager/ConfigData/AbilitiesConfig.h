#pragma once

#include "CoreMinimal.h"
#include "DataManager/EnumTypes/WarEnumTypes.h"
#include "Engine/DataAsset.h"
#include "AbilitiesConfig.generated.h"

class UGameplayAbility;
class UWarAbilitySystemComponent;

UCLASS()
class WAR_API UAbilitiesConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Abilities")
	TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>> DefaultAbilities;

	void GiveAbilities(const TObjectPtr<UWarAbilitySystemComponent>& ASC) const;
};
