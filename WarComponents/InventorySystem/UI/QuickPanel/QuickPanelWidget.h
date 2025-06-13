#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "War/WarComponents/InventorySystem/Interface/ContainerWidgetInterface.h"
#include "QuickPanelWidget.generated.h"

class USizeBox;
class UBorder;
class UHorizontalBox;
class UItemSlotWidget;

UCLASS()
class WAR_API UQuickPanelWidget : public UUserWidget, public IContainerWidgetInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	USizeBox* QuickSizeBox;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UBorder* QuickBorder;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category="UI")
	UHorizontalBox* QuickWrapBox;
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UItemSlotWidget> ItemSlotWidgetClass;
	UPROPERTY(VisibleAnywhere, Category="UI")
	TArray<TObjectPtr<UItemSlotWidget>> QuickSlots;
	UPROPERTY(VisibleAnywhere, Category="UI")
	int32 MaxSlots = 7;

public:
	virtual void InitSlots() override;
	virtual void SyncSlots() override;
	virtual void RemoveItemFromSlot(const FGuid& InID) override;
	virtual void ClearAllSlots() override;
	virtual void AddItemToSlot(const FGuid& InID) override;
	virtual void NativeConstruct() override;
	virtual int32 GetMaxSlots() const override { return MaxSlots; }
};
