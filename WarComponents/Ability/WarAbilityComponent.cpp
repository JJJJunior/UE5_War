#include "WarAbilityComponent.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "War/GameInstance/WarGameInstanceSubSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Types/WarDataTableType.h"
#include "War/Characters/Hero/WarHeroCharacter.h"


UWarAbilityComponent::UWarAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


TObjectPtr<UDataTable> UWarAbilityComponent::GetWarAbilityDataTable() const
{
	if (UWarGameInstanceSubSystem* SubSystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>())
	{
		return SubSystem->GetCachedWarAbilityDataTable();
	}
	return nullptr;
}

void UWarAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
	CachedOwnerCharacter = CastChecked<AWarHeroCharacter>(GetOwner());
	check(CachedOwnerCharacter);

	//初始化技能列表
	InitializeAbility();
}

//初始化技能列表，仅仅是把可以使用的table row的row name加载给能力列表。
void UWarAbilityComponent::InitializeAbility()
{
	if (AbilitiesToGive.IsEmpty()) return;

	for (const FName& ItemName : AbilitiesToGive)
	{
		FindAbilityRowAndSave(ItemName);
	}
}


//根据名称找到Row指针存下来
void UWarAbilityComponent::FindAbilityRowAndSave(const FName& RowName)
{
	if (RowName.IsNone()) return;
	for (const FName& ItemName : AbilitiesToGive)
	{
		if (ItemName == RowName)
		{
			if (FWarAbilityRow* ItemRow = GetWarAbilityDataTable()->FindRow<FWarAbilityRow>(RowName, "FindAbility"))
			{
				if (CurrentAbilityRowMap.Contains(RowName)) return;
				CurrentAbilityRowMap.Emplace(RowName, ItemRow);
			}
		}
	}
}

//播放声音
void UWarAbilityComponent::PlaySound(const FName& ItemRow, const TSoftObjectPtr<USoundBase>& SoundSoft)
{
	if (!CurrentAbilityRowMap.Contains(ItemRow)) return;
	if (SoundSoft.IsValid())
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundSoft.Get(), CachedOwnerCharacter->GetActorLocation());
	}
	else
	{
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		Streamable.RequestAsyncLoad(SoundSoft.ToSoftObjectPath(), [this,ItemRow]()
		{
			if (USoundBase* LoadedSound = CurrentAbilityRowMap[ItemRow]->AbilitySound.Get())
			{
				UGameplayStatics::PlaySoundAtLocation(this, LoadedSound, CachedOwnerCharacter->GetActorLocation());
			}
		});
	}
}

//设置攻击间隔
void UWarAbilityComponent::StartAttackCooldown(const float& CooldownTime)
{
	bCanAttack = false;

	FTimerHandle CooldownTimerHandle;
	CachedOwnerCharacter->GetWorldTimerManager().SetTimer(
		CooldownTimerHandle,
		this,
		&ThisClass::ResetAttackCooldown,
		CooldownTime,
		false
	);
}

//刷新攻击
void UWarAbilityComponent::ResetAttackCooldown()
{
	bCanAttack = true;
}


bool UWarAbilityComponent::IsValidComboAndRow(const FName& ItemRow, const int32& ComboIndex)
{
	return ComboIndex >= 0 && CurrentAbilityRowMap.Contains(ItemRow) && ComboIndex < CurrentAbilityRowMap[ItemRow]->WarAbilitySets.Num();
}


void UWarAbilityComponent::Attack()
{
	if (!bCanAttack) return;

	//选择技能使用表格动态加载，UseComboAttackIndex 是去row name 的索引
	const FName& ItemRow = AbilitiesToGive[UseComboAttackIndex];

	if (!IsValidComboAndRow(ItemRow, CurrentComboIndex)) return;
	PlayComboAnimation(ItemRow, CurrentComboIndex);
	StartAttackCooldown(CurrentAbilityRowMap[ItemRow]->WarAbilitySets[CurrentComboIndex].Interval);

	CurrentComboIndex++;
	if (CurrentComboIndex >= CurrentAbilityRowMap[ItemRow]->WarAbilitySets.Num())
	{
		CurrentComboIndex = 0;
	}
}


//根据列表中加载的数据索引播放攻击动画
void UWarAbilityComponent::PlayComboAnimation(const FName& ItemRow, const int32& ComboIndex)
{
	if (!CachedOwnerCharacter || !CachedOwnerCharacter->GetMesh())return;

	if (!IsValidComboAndRow(ItemRow, ComboIndex)) return;

	TSoftObjectPtr<UAnimMontage> MontageSoft = CurrentAbilityRowMap[ItemRow]->WarAbilitySets[ComboIndex].AnimMontage;

	UAnimInstance* AnimInstance = CachedOwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	if (MontageSoft.IsValid())
	{
		AnimInstance->Montage_Play(MontageSoft.Get());
	}
	else
	{
		TWeakObjectPtr<UAnimInstance> WeakAnimInstance = AnimInstance;
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		Streamable.RequestAsyncLoad(MontageSoft.ToSoftObjectPath(), [this,ItemRow,ComboIndex,WeakAnimInstance]()
		{
			if (!CurrentAbilityRowMap.Contains(ItemRow)) return;

			if (UAnimMontage* LoadedMontage = CurrentAbilityRowMap[ItemRow]->WarAbilitySets[ComboIndex].AnimMontage.Get())
			{
				if (WeakAnimInstance.IsValid())
				{
					WeakAnimInstance->Montage_Play(LoadedMontage);
				}
			}
		});
	}
}

// //用于切换连招序列
// void UWarAbilityComponent::ChangeComboAttackIndex()
// {
// 	//设定边界值：最大有效索引是 Num - 1
// 	if (UseComboAttackIndex < 0 || UseComboAttackIndex >= AbilitiesToGive.Num()) return;
//
// 	UseComboAttackIndex++;
// 	if (UseComboAttackIndex >= AbilitiesToGive.Num())
// 	{
// 		UseComboAttackIndex = 0;
// 	}
// }
