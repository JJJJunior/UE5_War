#pragma once

#include "CoreMinimal.h"
#include "WarDataTableType.generated.h"

USTRUCT(BlueprintType)
struct FWarAbilitySet
{
	GENERATED_BODY()

	FWarAbilitySet(): Damage(0), Consume(0), Interval(0)
	{
	};
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> AnimMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Damage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Consume;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Interval;
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
