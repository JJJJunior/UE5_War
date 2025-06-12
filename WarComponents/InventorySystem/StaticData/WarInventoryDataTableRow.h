#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WarInventoryDataTableRow.generated.h"


UENUM(BlueprintType)
enum class EEquipmentSlotType:uint8
{
	Head,
	Body,
	Neck,
	Legs,
	Footer,
	LeftHand,
	RightHand,
	Ring1,
	Ring2
};

UENUM(BlueprintType)
enum class EWarInventoryType : uint8
{
	Equipment, // 装备 (武器、护甲等可穿戴/使用的物品)
	QuestItem, // 任务物品 (专门用于任务的物品)
	Consumable, // 消耗品 (使用后会消失的物品，如药水、食物)
	Material // 材料 (用于合成的原材料)
};

class AInventoryBase;
class UAnimMontage;
class UTexture2D;

USTRUCT(BlueprintType)
struct FWarInventoryRow : public FTableRowBase
{
	GENERATED_BODY()

	FWarInventoryRow(): Texture(nullptr), InventoryType(), EquipmentSlotType()
	{
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Name;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Description;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftClassPtr<AInventoryBase> InventorySoftClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Texture;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EWarInventoryType InventoryType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EEquipmentSlotType EquipmentSlotType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName SocketName;
};
