#pragma once
#include "CoreMinimal.h"
#include "Tools/MyLog.h"
#include "UObject/Object.h"
#include "War/DataManager/EnumTypes/WarEnumTypes.h"
#include "InventoryData.generated.h"


USTRUCT(Blueprintable)
struct FWarSaveGameData
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid InstanceID = FGuid();

	UPROPERTY()
	int32 Count = 0;

	UPROPERTY()
	FName TableRowID;

	UPROPERTY()
	FSoftClassPath ActorClassPath = FSoftClassPath();

	UPROPERTY()
	FVector Location = FVector();

	UPROPERTY()
	FRotator Rotation = FRotator();

	UPROPERTY()
	FVector Scale = FVector();

	UPROPERTY()
	TArray<uint8> ActorData = TArray<uint8>(); //序列化

	// ⚙️ 是否动态生成（重要）
	UPROPERTY()
	bool bIsDynamicActor = false;

	//物体是否删除
	UPROPERTY()
	bool bIsDestroyed = false;

	FWarSaveGameData() = default;
};


USTRUCT()
struct FItemDataInSlot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category="UI")
	bool bIsFull = false;
	UPROPERTY(VisibleAnywhere, Category="UI")
	int32 MaxCount = 0;
	UPROPERTY(VisibleAnywhere, Category="UI")
	int32 Count = 0;
	UPROPERTY(VisibleAnywhere, Category="UI")
	FGuid InstanceID = FGuid();
	UPROPERTY(VisibleAnywhere, Category="UI")
	FName TableRowID = FName();
	UPROPERTY(VisibleAnywhere, Category="UI")
	TObjectPtr<UTexture2D> Texture = nullptr;
	UPROPERTY(VisibleAnywhere, Category="UI")
	ESlotType SlotType = ESlotType::None;

	FItemDataInSlot() = default;
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

	UPROPERTY(EditAnywhere)
	bool bIsEquipped = false;

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

	UPROPERTY(EditAnywhere)
	bool bIsEquipped = false;

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
		NewData.bIsEquipped = Params.bIsEquipped;
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
