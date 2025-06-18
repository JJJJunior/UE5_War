#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WarComponents/InventorySystem/StaticData/WarInventoryDataTableRow.h"
#include "ItemSlotWidget.generated.h"


class UBorder;
class UButton;
class UImage;
class UOverlay;
class USizeBox;
class UTextBlock;
class AInventoryBase;
class AWarHeroCharacter;

USTRUCT()
struct FItemDataInSlot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category="UI")
	bool bIsEmpty = true;
	UPROPERTY(VisibleAnywhere, Category="UI")
	int32 MaxCount = 0; //每个格子最大值
	UPROPERTY(VisibleAnywhere, Category="UI")
	int32 InventoryInSlots = 0; //当前物品数量
	UPROPERTY(VisibleAnywhere, Category="UI")
	FName CachedTableRowID;
	UPROPERTY(VisibleAnywhere, Category="UI")
	TArray<FGuid> InstanceIDs; //物品的实例ID
	UPROPERTY(VisibleAnywhere, Category="UI")
	TObjectPtr<UTexture2D> CachedTexture; //物品的图片
	UPROPERTY(VisibleAnywhere, Category="UI")
	int32 SlotIndex = 0;
	//用于判定父组件是哪一个
	UPROPERTY(VisibleAnywhere, Category="UI")
	FString ParentPanel;
	UPROPERTY(VisibleAnywhere, Category="UI")
	EEquipmentSlotType EquipmentSlotType;

	FItemDataInSlot(): CachedTexture(nullptr), EquipmentSlotType()
	{
	};
};


UCLASS()
class WAR_API UItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* MainSizeBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UBorder* Border1;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UBorder* Border2;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UOverlay* MainOverlay;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UImage* ItemImage;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* SizeBoxIn;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UTextBlock* ItemQuantity;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	FItemDataInSlot ItemDataInSlot;
	UPROPERTY()
	TWeakObjectPtr<AWarHeroCharacter> CachedCharacter;

	void SetMaxCount(const FGuid& InID);
	int32 GetMaxCount() const { return ItemDataInSlot.MaxCount; }


	virtual void NativeConstruct() override;
	void AddInventoryToSlot(const FGuid& InID);
	void RemoveItem();
	void RemoveItemByInstanceID(const FGuid& InID);
	void Show();
	void CleanSlot() const;
	FGuid GetFirstInstanceID(const FName& TableRowID) const;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
