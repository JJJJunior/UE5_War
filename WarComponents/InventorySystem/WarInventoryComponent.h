#pragma once
#include "CoreMinimal.h"
#include "War/DataManager/DynamicData/InventoryData.h"
#include "WarInventoryComponent.generated.h"


class AInventoryBase;
class URootPanelWidget;
class AWarHeroCharacter;

//广播通知UI打开状态
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIStateChanged, bool, bInventoryUIVisible);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WAR_API UWarInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	//广播通知UI打开状态
	void NotifyUIStateChanged() const
	{
		OnUIStateChanged.Broadcast(bInventoryUIVisible);
	}

protected:
	//初始化UI
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MySpawnData")
	TObjectPtr<URootPanelWidget> RootPanelWidget;

	UPROPERTY()
	TWeakObjectPtr<AWarHeroCharacter> CachedCharacter;
	//保存装备指针
	UPROPERTY()
	TMap<FGuid, TWeakObjectPtr<AInventoryBase>> SavedInventoryInSlots;

	void InitRootUI();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="MySpawnData")
	bool bInventoryUIVisible = false;

	// 装备物品（生成场景 Actor）
	void SpawnInventory(const FGuid& InInstanceID);
	void RollbackSpawnInventory(const FGuid& InInstanceID);
	static void DestroyEquippedInventory(const TWeakObjectPtr<AInventoryBase>& Inventory);

	//同步背包数据
	bool SyncJsonToBag() const;

	// 查询当前装备指针
	TWeakObjectPtr<AInventoryBase> FindSavedInventoryInSlots(const FGuid& InInstanceID) const;

public:
	UWarInventoryComponent();

	static bool CreateWeapon(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID);
	static bool CreateArmor(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID);
	static bool CreateConsumable(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID);
	static bool CreateQuestItem(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID);
	static bool CreateSkill(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID);
	static bool GenerateItemToBagAndSaved(const UObject* WorldContextObject, const FName& TableID, const FGuid& PlayerID);


	virtual void BeginPlay() override;
	void ToggleInventoryUI();
	// 提供状态查询函数
	bool IsInventoryUIVisible() const { return bInventoryUIVisible; }

	//给外部调用
	void EquipInventory(const FGuid& InInstanceID);
	void UnequipInventory(const FGuid& InInstanceID);
	bool HasInventoryInSomeSocket(const FGuid& InInstanceID, FGuid& FindID) const;
	
	//广播通知UI打开状态
	UPROPERTY()
	FOnUIStateChanged OnUIStateChanged;
	// UFUNCTION(BlueprintCallable)
	// bool GenerateItemToBagAndSaved(const FName& TableID);
	FORCEINLINE TObjectPtr<URootPanelWidget> GetRootPanelWidget() const { return RootPanelWidget; }
};
