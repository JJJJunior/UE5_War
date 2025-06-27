#include "InventoryPanelWidget.h"
#include "Characters/Hero/WarHeroCharacter.h"
#include "Components/WrapBox.h"
#include "War/WarComponents/InventorySystem/UI/ItemSlotWidget.h"
#include "Misc/StringBuilder.h"
#include "Tools/MyLog.h"
#include "Components/SizeBox.h"
#include "DataManager/ConfigData/GameConfigData.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "Kismet/GameplayStatics.h"
#include "WarComponents/PersistentSystem/WarPersistentSystem.h"

void UInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	checkf(ItemSlotWidgetClass, TEXT("ItemSlotWidgetClass 没有配置"));
	CachedCharacter = Cast<AWarHeroCharacter>(GetOwningPlayerPawn());
	if (!CachedCharacter.IsValid())
	{
		print(TEXT("CachedCharacter 弱指针无效"));
		return;
	}
	MaxSlots = UWarGameInstanceSubSystem::GetGameConfigData(this)->MaxSlots;
	InitSlots();
}

// 初始化背包槽位
void UInventoryPanelWidget::InitSlots()
{
	TMap<ESlotType, USizeBox*> SlotBoxMap = {
		{ESlotType::Head, HeadBox},
		{ESlotType::Body, BodyBox},
		{ESlotType::LeftHand, LeftHandBox},
		{ESlotType::RightHand, RightHandBox},
		{ESlotType::Legs, LegBox},
		{ESlotType::Footer, FooterBox},
		{ESlotType::Neck, NeckBox},
		{ESlotType::Ring1, RingBox1},
		{ESlotType::Ring2, RingBox2},
	};

	int32 index = 0;

	for (const auto& Pair : SlotBoxMap)
	{
		ESlotType SlotType = Pair.Key;
		USizeBox* SlotBox = Pair.Value;

		if (!SlotBox) continue;

		if (UItemSlotWidget* SlotWidget = CreateWidget<UItemSlotWidget>(GetWorld(), ItemSlotWidgetClass))
		{
			//设置索引
			SlotWidget->SetVisibility(ESlateVisibility::Visible);
			SlotWidget->InitSlot(SlotType);
			SlotBox->AddChild(SlotWidget);
			CharacterSlots.Emplace(SlotType, SlotBox);
		}
		index++;
	}

	if (!InventoryWrapBox) return;
	for (int32 i = 0; i < MaxSlots; i++)
	{
		if (UItemSlotWidget* SlotWidget = CreateWidget<UItemSlotWidget>(GetWorld(), ItemSlotWidgetClass))
		{
			//设置索引
			SlotWidget->SetVisibility(ESlateVisibility::Visible);
			SlotWidget->InitSlot(ESlotType::Normal);
			InventoryWrapBox->AddChildToWrapBox(SlotWidget);
			InventorySlots.Add(SlotWidget);
		}
	}
}


// 同步背包数据
void UInventoryPanelWidget::SyncSlots()
{
	UWarGameInstanceSubSystem* Subsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return;
	UWarPersistentSystem* PersistentSystem = Subsystem->GetWarPersistentSystem();
	if (!PersistentSystem) return;

	TArray<FInventoryItemInDB> ItemInDBArray;
	PersistentSystem->FindAllInventoriesByPlayerID(CachedCharacter->GetPersistentID(), ItemInDBArray);

	if (ItemInDBArray.Num() > 0)
	{
		for (const FInventoryItemInDB& Inventory : ItemInDBArray)
		{
			AddItemToSlot(Inventory);
		}
	}
}


// 清空所有槽位
void UInventoryPanelWidget::ClearAllSlots()
{
	for (const TObjectPtr<UItemSlotWidget>& InSlot : InventorySlots)
	{
		if (InSlot)
		{
			InSlot->CleanSlot(); // 这里改成清空整个槽位数据
		}
	}
}

// 添加物品（支持堆叠）
void UInventoryPanelWidget::AddItemToSlot(const FInventoryItemInDB& InItemInDB)
{
	if (UItemSlotWidget* FindSlot = FindSuitableSlot(InItemInDB))
	{
		FindSlot->AddToSlot(InItemInDB);
	}
}


// 查找同类型且未满的格子
UItemSlotWidget* UInventoryPanelWidget::FindSuitableSlot(const FInventoryItemInDB& InItemInDB)
{
	const bool bCanStack = InItemInDB.InventoryType == EWarInventoryType::Consumable ||
		InItemInDB.InventoryType == EWarInventoryType::QuestItem ||
		InItemInDB.InventoryType == EWarInventoryType::Material;

	if (bCanStack)
	{
		// 1. 查找已有未满的堆叠格子
		for (const TObjectPtr<UItemSlotWidget>& InSlot : InventorySlots)
		{
			if (!InSlot->SlotData.bIsFull && InSlot->bIsInitialized && InSlot->SlotData.TableRowID == InItemInDB.TableRowID)
			{
				return InSlot;
			}
		}

		// 2. 没有堆叠目标？找空格子
		for (const TObjectPtr<UItemSlotWidget>& InSlot : InventorySlots)
		{
			if (!InSlot->SlotData.bIsFull && InSlot->bIsInitialized && InSlot->SlotData.TableRowID.IsNone())
			{
				return InSlot;
			}
		}
	}
	else
	{
		// 不可堆叠：找空格子
		for (const TObjectPtr<UItemSlotWidget>& InSlot : InventorySlots)
		{
			if (!InSlot->SlotData.bIsFull &&
				InSlot->bIsInitialized &&
				InSlot->SlotData.TableRowID.IsNone())
			{
				return InSlot;
			}
		}
	}
	return nullptr;
}


// 从背包移除装备
void UInventoryPanelWidget::RemoveItemFromSlot(const FInventoryItemInDB& InItemInDB)
{
	for (const TObjectPtr<UItemSlotWidget>& InSlot : InventorySlots)
	{
		if (InSlot->SlotData.Count == 0) continue;

		// 判断这个格子的 InstanceIDs 是否包含目标实例 ID
		if (InSlot->SlotData.InstanceID == InItemInDB.InstanceID && InItemInDB.Count > 0)
		{
			// 执行移除
			InSlot->RemoveFromSlot(InItemInDB);
			break; // 找到立即退出
		}
	}
}
