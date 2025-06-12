#pragma once


#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventoryInstanceData.generated.h"


class UInventoryExtraData;
class AInventoryBase;
enum class EWarInventoryType : uint8;


USTRUCT(Blueprintable)
struct FInventoryInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid InstanceID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TableRowID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWarInventoryType InventoryType; // 武器、装备、消耗品、任务物品等
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInventoryExtraData> ExtraData; // 子类实例数据，支持多态

	FInventoryInstanceData(): InventoryType()
	{
		InstanceID = FGuid::NewGuid();
	};
};

UCLASS(Blueprintable)
class UInventoryExtraData : public UObject
{
	GENERATED_BODY()
};

UCLASS(Blueprintable)
class UEquipmentInstanceData : public UInventoryExtraData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Durability;
};

UCLASS(Blueprintable)
class UWeaponInstanceData : public UEquipmentInstanceData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackPower;

	void Initialize(float InDurability, float InAttackPower)
	{
		Durability = InDurability;
		AttackPower = InAttackPower;
	}

	//工厂函数
	static TObjectPtr<UWeaponInstanceData> CreateWeaponInstance(UObject* Outer, float InDurability, float InAttackPower)
	{
		TObjectPtr<UWeaponInstanceData> NewInstance = NewObject<UWeaponInstanceData>(Outer);
		NewInstance->Initialize(InDurability, InAttackPower);
		return NewInstance;
	}
};


UCLASS(Blueprintable)
class UConsumableInstanceData : public UInventoryExtraData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsQuestRelated;
};


UCLASS(Blueprintable)
class UQuestItemInstanceData : public UInventoryExtraData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCollected;
};
