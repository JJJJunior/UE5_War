#pragma once

#include "CoreMinimal.h"
#include "WarDataTableType.generated.h"

USTRUCT(BlueprintType)
struct FWarAbilitySet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> AnimMontage = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Damage = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Consume = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Interval = 0.0f;

	FWarAbilitySet() = default;
};


USTRUCT(BlueprintType)
struct FWarAbilityRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Name;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Description;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FWarAbilitySet> WarAbilitySets;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> AbilitySound;
};
