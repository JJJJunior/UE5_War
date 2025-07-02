#include "KatanaComboAbilityBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "DataManager/StaticData/WarDataTableType.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameInstance/WarGameInstanceSubSystem.h"


UKatanaComboAbilityBase::UKatanaComboAbilityBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bReplicateInputDirectly = true;
	bServerRespectsRemoteAbilityCancellation = true;

	// 关键：必须开启输入响应
	bRespondToInput = true;
}


void UKatanaComboAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CurrentSpecHandle = Handle;
	CurrentActorInfo = ActorInfo;
	CurrentActivationInfo = ActivationInfo;

	CurrentComboIndex = 0;
	bCanAttack = true;
}

void UKatanaComboAbilityBase::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (!bCanAttack) return;

	if (KatanaComboName.IsNone()) return;

	// ComboIndex 随次数累加
	Attack(KatanaComboName);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("KatanaCombo %d"), CurrentComboIndex));
	}
}


void UKatanaComboAbilityBase::Attack(const FName& TableRowID)
{
	const FWarAbilityRow* FindRow = UWarGameInstanceSubSystem::FindAbilityRow(this, TableRowID);
	if (!FindRow || !FindRow->WarAbilitySets.IsValidIndex(CurrentComboIndex)) return;

	// 播放当前段
	PlayComboAnimation(TableRowID, CurrentComboIndex);

	// 设置冷却
	bCanAttack = false;
	StartAttackCooldown(FindRow->WarAbilitySets[CurrentComboIndex].Interval);

	// 下一段准备
	CurrentComboIndex++;

	// 如果超过最大段数就重置
	if (CurrentComboIndex >= FindRow->WarAbilitySets.Num())
	{
		CurrentComboIndex = 0;
		// 技能可以结束
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UKatanaComboAbilityBase::PlayComboAnimation(const FName& TableRowID, int32 ComboIndex) const
{
	// 获取角色
	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !Character->GetMesh()) return;

	// 获取动画数据行
	const FWarAbilityRow* FindRow = UWarGameInstanceSubSystem::FindAbilityRow(this, TableRowID);
	if (!FindRow || !FindRow->WarAbilitySets.IsValidIndex(ComboIndex)) return;

	const FWarAbilitySet& AbilitySet = FindRow->WarAbilitySets[ComboIndex];
	TSoftObjectPtr<UAnimMontage> MontageSoft = AbilitySet.AnimMontage;

	// 获取动画实例
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	if (MontageSoft.IsValid())
	{
		// 如果已经加载，直接播放
		AnimInstance->Montage_Play(MontageSoft.Get());
	}
	else
	{
		// 否则异步加载播放
		TWeakObjectPtr<UAnimInstance> WeakAnimInstance = AnimInstance;
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		FSoftObjectPath SoftPath = MontageSoft.ToSoftObjectPath();

		Streamable.RequestAsyncLoad(SoftPath, [WeakAnimInstance, SoftPath]()
		{
			if (!WeakAnimInstance.IsValid()) return;

			UAnimMontage* LoadedMontage = Cast<UAnimMontage>(SoftPath.ResolveObject());
			if (!LoadedMontage)
			{
				LoadedMontage = Cast<UAnimMontage>(SoftPath.TryLoad());
			}

			if (LoadedMontage)
			{
				WeakAnimInstance->Montage_Play(LoadedMontage);
			}
		});
	}
}


void UKatanaComboAbilityBase::StartAttackCooldown(float CooldownTime)
{
	AWarHeroCharacter* Character = Cast<AWarHeroCharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return;

	FTimerHandle CooldownTimerHandle;
	Character->GetWorldTimerManager().SetTimer(CooldownTimerHandle, this, &UKatanaComboAbilityBase::ResetAttackCooldown, CooldownTime, false);
}

void UKatanaComboAbilityBase::ResetAttackCooldown()
{
	bCanAttack = true;
}


void UKatanaComboAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                                         bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
