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


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WAR_API UWarInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MySpawnData")
	TObjectPtr<AWarCharacterBase> CachedOwnerCharacter;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MySpawnData")
	TObjectPtr<APlayerController> CachedOwnerController;
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
	TMap<FGuid, TWeakObjectPtr<AInventoryBase>> InstanceToActorMap;
	//快捷栏
	UPROPERTY()
	TArray<FGuid> QuickSlots;

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
	const FInventoryInstanceData* GetInventoryDataByGuid(const FGuid& Guid) const;

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
	FGuid HasInventoryInSocket(const FGuid& InID) const;
	// 查询当前装备的场景 Actor
	TWeakObjectPtr<AInventoryBase> GetSceneActor(const FGuid& InstanceID) const;
	TObjectPtr<AInventoryBase> FindActorInSocket(const FGuid& InID) const;

	void ShowCurrentInventories() const;
};
