#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "KatanaComboAbilityBase.generated.h"


UCLASS()
class WAR_API UKatanaComboAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Abilities")
	FName KatanaComboName;
	UPROPERTY()
	int32 CurrentComboIndex = 0;
	UPROPERTY()
	bool bCanAttack = true;
	UPROPERTY()
	bool bRespondToInput = false;

	void Attack(const FName& TableRowID);
	void StartAttackCooldown(float CooldownTime);
	void ResetAttackCooldown();

	void PlayComboAnimation(const FName& TableRowID, int32 ComboIndex) const;

public:
	UKatanaComboAbilityBase();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                          const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
