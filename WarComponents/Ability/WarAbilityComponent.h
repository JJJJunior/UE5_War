#pragma once

#include "CoreMinimal.h"
#include "War/Types/WarDataTableType.h"
#include "WarAbilityComponent.generated.h"


class AWarHeroCharacter;
class AWarAbilityComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WAR_API UWarAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MyAbilityData")
	TObjectPtr<AWarHeroCharacter> CachedOwnerCharacter;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MyAbilityData")
	TArray<FName> AbilitiesToGive;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MyAbilityData")
	bool bCanAttack = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MyAbilityData")
	int32 CurrentComboIndex = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MyAbilityData")
	int32 UseComboAttackIndex = 0;

	TMap<FName, const FWarAbilityRow*> CurrentAbilityRowMap;
	TObjectPtr<UDataTable> GetWarAbilityDataTable() const;
	void PlayComboAnimation(const FName& ItemRow, const int32& ComboIndex);
	void FindAbilityRowAndSave(const FName& RowName);
	void PlaySound(const FName& ItemRow, const TSoftObjectPtr<USoundBase>& SoundSoft);
	bool IsValidComboAndRow(const FName& ItemRow, const int32& ComboIndex);

	void StartAttackCooldown(const float& CooldownTime);
	void ResetAttackCooldown();

	void InitializeAbility();

public:
	UWarAbilityComponent();
	virtual void BeginPlay() override;
	void Attack();
};
