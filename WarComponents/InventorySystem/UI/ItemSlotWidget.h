#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "War/DataManager/DynamicData/InventoryData.h"
#include "ItemSlotWidget.generated.h"


class UBorder;
class UButton;
class UImage;
class UOverlay;
class USizeBox;
class UTextBlock;
class AWarHeroCharacter;
class UWarPersistentSystem;


UCLASS()
class WAR_API UItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UImage* ItemImage;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UTextBlock* ItemQuantity;
	UPROPERTY()
	TWeakObjectPtr<AWarHeroCharacter> CachedCharacter;
	UPROPERTY()
	TWeakObjectPtr<UWarPersistentSystem> CachedPersistentSystem;
	void Show() const;
	void SetMaxCount(const FInventoryItemInDB& ItemInDB);

public:
	bool bIsInitialized = false;
	UPROPERTY(VisibleAnywhere, Category="UI")
	FItemDataInSlot SlotData;
	//初始化函数
	virtual void NativeConstruct() override;
	FORCEINLINE int32 GetMaxCount() const { return SlotData.MaxCount; }
	void InitSlot(const ESlotType& InSlotType);
	bool AddToSlot(const FInventoryItemInDB& ItemInDB);
	void RemoveFromSlot(const FInventoryItemInDB& ItemInDB);
	void CleanSlot();

	virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
