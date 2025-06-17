#pragma once

#include "CoreMinimal.h"
#include "War/WarComponents/InventorySystem/DynamicData/InventoryInstanceData.h"
#include "WarInventoryComponent.generated.h"


class UGameInstanceSubsystem;
class AWarCharacterBase;
class AInventoryBase;
class APlayerController;
struct FWarSpawnInventoryRow;
class UUserWidget;
class URootPanelWidget;
class UInventoryPanelWidget;
class UCharacterPanelWidget;

//广播通知UI打开状态
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUIStateChanged, bool, bCharacterUIVisible, bool, bInventoryUIVisible);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WAR_API UWarInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	//广播通知UI打开状态
	void NotifyUIStateChanged() const
	{
		OnUIStateChanged.Broadcast(bCharacterUIVisible, bInventoryUIVisible);
	}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MySpawnData")
	TObjectPtr<AWarCharacterBase> CachedOwnerCharacter;
	TObjectPtr<UDataTable> GetInventoryDataTable() const;
	//初始化UI
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MySpawnData")
	TSubclassOf<URootPanelWidget> RootPanelWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="MySpawnData")
	TObjectPtr<URootPanelWidget> RootPanelWidget;

	//全局物品数据池
	UPROPERTY()
	TMap<FGuid, FInventoryInstanceData> AllInventoryData;
	//当前背包
	UPROPERTY()
	TSet<FGuid> CurrentInInventories;
	//当前装备的ID
	UPROPERTY()
	TSet<FGuid> CurrentEquippedItems;
	// ID -> 场景 Actor 映射表（用 WeakPtr 防止内存泄漏）
	UPROPERTY()
	TMap<FGuid, TObjectPtr<AInventoryBase>> InstanceToActorMap;
	//快捷栏
	UPROPERTY()
	TSet<FGuid> CurrentInQuickItems;

	void InitRootUI();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="MySpawnData")
	bool bInventoryUIVisible = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="MySpawnData")
	bool bCharacterUIVisible = false;

public:
	UWarInventoryComponent();
	virtual void BeginPlay() override;
	TSet<FGuid> GetCurrentEquippedItems() const { return CurrentEquippedItems; }
	TSet<FGuid> GetCurrentInInventories() const { return CurrentInInventories; }
	TSet<FGuid> GetCurrentInQuickItems() const { return CurrentInQuickItems; }
	const FInventoryInstanceData* FindInventoryDataByGuid(const FGuid& Guid) const;

	void ToggleInventoryUI();
	void ToggleCharacterUI();
	FInventoryInstanceData GenerateNewWeapon(const FName& InInventoryName);

	void InitInventories();
	//添加物品
	void AddInventory(const FInventoryInstanceData& NewData);
	// 装备物品（生成场景 Actor）
	void EquipInventory(const FGuid& InID);
	void UnequipInventory(const FGuid& InID);
	void SpawnInventory(const FGuid& InID);
	bool HasInventoryInSocket(const FGuid& InID) const;
	// 查询当前装备的场景 Actor
	TObjectPtr<AInventoryBase> FindActorInActorMap(const FGuid& InID) const;

	void ShowCurrentInventories() const;
	
	//广播通知UI打开状态
	UPROPERTY()
	FOnUIStateChanged OnUIStateChanged;
	// 提供状态查询函数
	bool IsInventoryUIVisible() const { return bInventoryUIVisible; }
	bool IsCharacterUIVisible() const { return bCharacterUIVisible; }
};
