#pragma once

#include "CoreMinimal.h"
#include "War/DataManager/DynamicData/InventoryData.h"
#include "WarInventoryComponent.generated.h"


class UGameInstanceSubsystem;
class AWarCharacterBase;
class AInventoryBase;
class APlayerController;
class UUserWidget;
class URootPanelWidget;
class UInventoryPanelWidget;
class UCharacterPanelWidget;
class USizeBox;

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
	UPROPERTY()
	TWeakObjectPtr<AWarCharacterBase> CachedOwnerCharacter;
	//初始化UI
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MySpawnData")
	TSubclassOf<URootPanelWidget> RootPanelWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="MySpawnData")
	TObjectPtr<URootPanelWidget> RootPanelWidget;

	//背包栏
	UPROPERTY()
	TArray<FItemInBagData> Inventories;
	//装备栏
	UPROPERTY()
	TArray<FItemInBagData> EquippedItems;
	//快捷栏
	UPROPERTY()
	TArray<FItemInBagData> QuickItems;

	//保存装备指针
	UPROPERTY()
	TMap<FGuid, TWeakObjectPtr<AInventoryBase>> SavedInventoryInSlots;


	void InitRootUI();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="MySpawnData")
	bool bInventoryUIVisible = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="MySpawnData")
	bool bCharacterUIVisible = false;

	//添加物品
	void AddInventory(const FItemInBagData& InBagData);
	// 装备物品（生成场景 Actor）
	void SpawnInventory(const FItemInBagData& InBagData);
	bool HasInventoryInSocket(const FItemInBagData& InBagData) const;
	// 查询当前装备指针
	TWeakObjectPtr<AInventoryBase> FindSavedInventoryInSlots(const FItemInBagData& InBagData) const;
	void ShowCurrentInventories() const;
	//根据Guid找数据
	TOptional<FItemInBagData> FindItemBagDataFromEquipped(const FGuid& InID);

public:
	UWarInventoryComponent();
	virtual void BeginPlay() override;
	const TArray<FItemInBagData>& GetCurrentEquippedItems() const { return EquippedItems; }
	const TArray<FItemInBagData>& GetCurrentInInventories() const { return Inventories; }
	const TArray<FItemInBagData>& GetCurrentInQuickItems() const { return QuickItems; }
	void ToggleInventoryUI();
	void ToggleCharacterUI();

	// 提供状态查询函数
	bool IsInventoryUIVisible() const { return bInventoryUIVisible; }
	bool IsCharacterUIVisible() const { return bCharacterUIVisible; }

	//给外部调用
	//穿装备
	void EquipInventory(const FItemInBagData& InBagData);
	void UnequipInventory(const FItemInBagData& InBagData);
	//广播通知UI打开状态
	UPROPERTY()
	FOnUIStateChanged OnUIStateChanged;
	bool GenerateItemToBagAndSaved(const FName& TableID);
	FORCEINLINE TObjectPtr<URootPanelWidget> GetRootPanelWidget() const { return RootPanelWidget; }
};
