#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventoryInstanceData.generated.h"


class UInventoryExtraData;
class AInventoryBase;


UENUM(BlueprintType)
enum class EWarInventoryType: uint8
{
	None = 0,
	Equipment, // 装备 (武器、护甲等可穿戴/使用的物品)
	QuestItem, // 任务物品 (专门用于任务的物品)
	Consumable, // 消耗品 (使用后会消失的物品，如药水、食物)
	Material, // 材料 (用于合成的原材料)
	Skill //技能用于施法
};


USTRUCT(Blueprintable)
struct FEquipmentData
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float Amount = 0.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Defense = 0.0f;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float Durability = 0.0f;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FGuid PlayerID = FGuid();
};


USTRUCT(Blueprintable)
struct FQuestItemData
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FGuid QuestID = FGuid();
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FGuid PlayerID = FGuid();
};


USTRUCT(BlueprintType)
struct FInventoryCreateParams
{
	GENERATED_BODY()
	//基础参数
	UPROPERTY(EditAnywhere)
	FGuid InstanceID = FGuid::NewGuid();
	UPROPERTY(EditAnywhere)
	FGuid PlayerID = FGuid();
	UPROPERTY(EditAnywhere)
	FName TableRowID = FName();
	UPROPERTY(EditAnywhere)
	float Count = 0;
	UPROPERTY(EditAnywhere)
	EWarInventoryType InventoryType = EWarInventoryType::None;
	// 装备参数
	UPROPERTY(EditAnywhere)
	float Defense = 0.0f;
	UPROPERTY(EditAnywhere)
	float Amount = 0.0f;
	UPROPERTY(EditAnywhere)
	float Durability = 0.0f;
	// 任务参数
	UPROPERTY(EditAnywhere)
	FGuid QuestID = FGuid();
};

USTRUCT(Blueprintable)
struct FInventoryInstanceData
{
	GENERATED_BODY()
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FGuid InstanceID;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FGuid PlayerID;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FName TableRowID;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	int32 Count = 0;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	EWarInventoryType InventoryType = EWarInventoryType::None; // 武器、装备、消耗品、任务物品等
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FEquipmentData EquipmentData;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FQuestItemData QuestItemData;

	//禁用默认构造函数，强制使用工厂
	FInventoryInstanceData() = default;

	static FInventoryInstanceData CreateInventoryData(const FInventoryCreateParams& Params)
	{
		FInventoryInstanceData NewInstance;

		//--- 基础数据初始化 ---
		// 如果调用方未提供InstanceID，自动生成
		NewInstance.InstanceID = Params.InstanceID;
		NewInstance.PlayerID = Params.PlayerID;
		NewInstance.TableRowID = Params.TableRowID;
		NewInstance.Count = FMath::Max(0, Params.Count); // 确保至少为0
		NewInstance.InventoryType = Params.InventoryType;
		//--- 类型特定数据初始化 ---
		switch (Params.InventoryType)
		{
		case EWarInventoryType::Equipment:
			NewInstance.EquipmentData = FEquipmentData{
				Params.Amount,
				Params.Defense,
				FMath::Max(0, Params.Durability),
				Params.PlayerID
			};
			break;

		case EWarInventoryType::QuestItem:
			NewInstance.QuestItemData = FQuestItemData{
				Params.QuestID,
				Params.PlayerID
			};
			break;

		default:
			UE_LOG(LogTemp, Error, TEXT("不支持的 InventoryType: %d"), static_cast<int32>(Params.InventoryType));
			return FInventoryInstanceData(); // 返回全默认值结构体
		}
		return NewInstance;
	}
};
