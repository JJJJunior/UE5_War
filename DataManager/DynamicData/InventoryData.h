#pragma once
#include "CoreMinimal.h"
#include "Tools/MyLog.h"
#include "UObject/Object.h"
#include "War/DataManager/EnumTypes/WarEnumTypes.h"
#include "InventoryData.generated.h"


USTRUCT(BlueprintType)
struct FItemInBagData
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid InstanceID = FGuid();
	UPROPERTY()
	FName TableRowID = FName();
	UPROPERTY()
	EWarInventoryType InventoryType = EWarInventoryType::None;
	UPROPERTY()
	int32 Count = 0;

	FItemInBagData() = default;

	static FItemInBagData CreateInBagData(const FGuid& InstanceID, const FName& TableRowID, const int32 Count, const EWarInventoryType& InventoryType)
	{
		FItemInBagData NewData;
		NewData.InstanceID = InstanceID;
		NewData.TableRowID = TableRowID;
		NewData.InventoryType = InventoryType;
		NewData.Count = Count;
		return NewData;
	}

	bool operator==(const FItemInBagData& Other) const
	{
		return InventoryType == Other.InventoryType && InstanceID == Other.InstanceID && TableRowID == Other.TableRowID;
	}
};


USTRUCT(BlueprintType)
struct FInventoryItemInDBParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGuid InstanceID = FGuid();

	UPROPERTY(EditAnywhere)
	FGuid PlayerID = FGuid();

	UPROPERTY(EditAnywhere)
	FName TableRowID = FName();

	UPROPERTY(EditAnywhere)
	int32 Count = 0;

	UPROPERTY(EditAnywhere)
	int32 Level = 0;

	UPROPERTY(VisibleDefaultsOnly)
	float Cooldown = 0.0f;

	// 公共参数
	UPROPERTY(EditAnywhere)
	float Durability = 0.0f;

	UPROPERTY(EditAnywhere)
	EWarInventoryType InventoryType = EWarInventoryType::None;

	// 武器专用
	UPROPERTY(EditAnywhere)
	float Damage = 0.0f;

	// 护甲专用
	UPROPERTY(EditAnywhere)
	float Defense = 0.0f;

	// 任务物品专用
	UPROPERTY(EditAnywhere)
	FGuid QuestID = FGuid();

	//消耗品专用
	UPROPERTY(EditAnywhere)
	float Amount = 0.0f;

	FInventoryItemInDBParams() = default;
};

USTRUCT(BlueprintType)
struct FInventoryItemInDB
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGuid InstanceID = FGuid();

	UPROPERTY(EditAnywhere)
	FGuid PlayerID = FGuid();

	UPROPERTY(EditAnywhere)
	FName TableRowID = FName();

	UPROPERTY(EditAnywhere)
	int32 Count = 0;

	UPROPERTY(EditAnywhere)
	int32 Level = 0;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float Cooldown = 0.0f;

	// 公共参数
	UPROPERTY(EditAnywhere)
	float Durability = 0.0f;

	UPROPERTY(EditAnywhere)
	EWarInventoryType InventoryType = EWarInventoryType::None;

	// 武器专用
	UPROPERTY(EditAnywhere)
	float Damage = 0.0f;

	// 护甲专用
	UPROPERTY(EditAnywhere)
	float Defense = 0.0f;

	// 任务物品专用
	UPROPERTY(EditAnywhere)
	FGuid QuestID = FGuid();

	//消耗品专用
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float Amount = 0.0f;

	FInventoryItemInDB() = default;

	static FInventoryItemInDB Init(const FInventoryItemInDBParams& Params)
	{
		FInventoryItemInDB NewData;
		NewData.InstanceID = Params.InstanceID;
		NewData.PlayerID = Params.PlayerID;
		NewData.TableRowID = Params.TableRowID;
		NewData.InventoryType = Params.InventoryType;
		NewData.Count = Params.Count;
		NewData.Level = Params.Level;
		NewData.Cooldown = Params.Cooldown;
		NewData.Durability = Params.Durability;
		NewData.Damage = Params.Damage;
		NewData.Defense = Params.Defense;
		NewData.QuestID = Params.QuestID;
		return NewData;
	}

	bool operator==(const FInventoryItemInDB& Other) const
	{
		return InstanceID == Other.InstanceID;
	}
};
