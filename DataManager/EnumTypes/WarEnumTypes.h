#pragma once

#include "CoreMinimal.h"


UENUM(BlueprintType)
enum class EWarInventoryType: uint8
{
	None = 0,
	Weapon, // 武器
	Armor, //装备
	QuestItem, // 任务物品 (专门用于任务的物品)
	Consumable, // 消耗品 (使用后会消失的物品，如药水、食物)
	Material, // 材料 (用于合成的原材料)
	Skill //技能用于施法
};


UENUM(BlueprintType)
enum class ESlotType:uint8
{
	None = 0,
	Head,
	Body,
	Neck,
	Legs,
	Footer,
	LeftHand,
	RightHand,
	Ring1,
	Ring2,
	Skill,
	Normal
};
