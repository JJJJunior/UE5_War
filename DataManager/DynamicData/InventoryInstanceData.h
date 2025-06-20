#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "War/DataManager/EnumTypes/WarEnumTypes.h"
#include "InventoryInstanceData.generated.h"


USTRUCT(BlueprintType)
struct FCreateInventoryDataParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGuid InstanceID = FGuid();

	UPROPERTY(EditAnywhere)
	FGuid PlayerID = FGuid();

	UPROPERTY(EditAnywhere)
	FName TableRowID = FName();

	UPROPERTY(EditAnywhere)
	float Count = 0;

	// 公共参数
	UPROPERTY(EditAnywhere)
	float Durability = 100.0f;

	UPROPERTY(EditAnywhere)
	EWarInventoryType InventoryType = EWarInventoryType::None;

	// 武器专用
	UPROPERTY(EditAnywhere)
	float Damage = 10.0f;

	// 护甲专用
	UPROPERTY(EditAnywhere)
	float Defense = 5.0f;

	// 任务物品专用
	UPROPERTY(EditAnywhere)
	FGuid QuestID = FGuid();

	//消耗品专用
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float Amount = 0.0f;

	//技能专用
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	int32 Level = 1;
};

USTRUCT(Blueprintable)
struct FInventoryItemData
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FGuid InstanceID = FGuid();

	UPROPERTY(EditAnywhere)
	FGuid PlayerID = FGuid();

	UPROPERTY(EditAnywhere)
	FName TableRowID = FName();

	UPROPERTY(EditAnywhere)
	float Count = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	EWarInventoryType InventoryType = EWarInventoryType::None;

	virtual ~FInventoryItemData() = default;

	virtual void Init(const FCreateInventoryDataParams& Params)
	{
		InstanceID = Params.InstanceID;
		PlayerID = Params.PlayerID;
		TableRowID = Params.TableRowID;
		InventoryType = Params.InventoryType;
		Count = Params.Count;
	}
};

// 武器数据
USTRUCT(Blueprintable)
struct FWeaponData : public FInventoryItemData
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float Damage = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float Durability = 0.0f;

	virtual void Init(const FCreateInventoryDataParams& Params) override
	{
		FInventoryItemData::Init(Params);
		Damage = Params.Damage;
		Durability = Params.Durability;
	}

	// 在FWeaponData中添加：
	static EWarInventoryType StaticInventoryType() { return EWarInventoryType::Weapon; }
};

// 护甲数据
USTRUCT(Blueprintable)
struct FArmorData : public FInventoryItemData
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float Defense = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float Durability = 0.0f;

	virtual void Init(const FCreateInventoryDataParams& Params) override
	{
		FInventoryItemData::Init(Params);
		Defense = Params.Defense;
		Durability = Params.Durability;
	}

	// 在FArmorData中添加：
	static EWarInventoryType StaticInventoryType() { return EWarInventoryType::Armor; }
};

// 任务物品数据
USTRUCT(Blueprintable)
struct FQuestItemData : public FInventoryItemData
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FGuid QuestID = FGuid();

	virtual void Init(const FCreateInventoryDataParams& Params) override
	{
		FInventoryItemData::Init(Params);
		QuestID = Params.QuestID;
	}

	// 在FQuestItemData中添加：
	static EWarInventoryType StaticInventoryType() { return EWarInventoryType::QuestItem; }
};

// 任务物品数据
USTRUCT(Blueprintable)
struct FConsumableData : public FInventoryItemData
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float Amount = 0.0f;

	virtual void Init(const FCreateInventoryDataParams& Params) override
	{
		FInventoryItemData::Init(Params);
		Amount = Params.Amount;
	}

	static EWarInventoryType StaticInventoryType() { return EWarInventoryType::Consumable; }
};

// 技能物品
USTRUCT(Blueprintable)
struct FSkillData : public FInventoryItemData
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	int32 Level = 1;

	virtual void Init(const FCreateInventoryDataParams& Params) override
	{
		FInventoryItemData::Init(Params);
		Level = Params.Level;
	}

	static EWarInventoryType StaticInventoryType() { return EWarInventoryType::Skill; }
};


// 工厂函数：创建物品数据
static TSharedPtr<FInventoryItemData> CreateInventoryItem(const FCreateInventoryDataParams& Params)
{
	TSharedPtr<FInventoryItemData> Item;
	switch (Params.InventoryType)
	{
	case EWarInventoryType::Weapon:
		Item = MakeShared<FWeaponData>();
		break;
	case EWarInventoryType::Armor:
		Item = MakeShared<FArmorData>();
		break;
	case EWarInventoryType::QuestItem:
		Item = MakeShared<FQuestItemData>();
		break;
	case EWarInventoryType::Consumable:
		Item = MakeShared<FConsumableData>();
		break;
	case EWarInventoryType::Skill:
		Item = MakeShared<FSkillData>();
		break;
	default:
		return nullptr;
	}
	Item->Init(Params);
	return Item;
}
