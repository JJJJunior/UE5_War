#include "RootPanelWidget.h"
#include "Components/WrapBox.h"
#include "War/WarComponents/InventorySystem/UI/CharacterPanel/CharacterPanelWidget.h"
#include "War/WarComponents/InventorySystem/UI/InventoryPanel/InventoryPanelWidget.h"


void URootPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	checkf(InventoryPanelWidgetClass, TEXT("InventoryPanelWidgetClass 没有配置"));
	checkf(CharacterPanelWidgetClass, TEXT("CharacterPanelWidgetClass 没有配置"));


	//初始化UI背包
	InventoryPanelWidget = CreateWidget<UInventoryPanelWidget>(GetWorld(), InventoryPanelWidgetClass);
	if (InventoryPanelWidget)
	{
		InventoryPanelWidget->SetVisibility(ESlateVisibility::Hidden);
		InventoryWrapBox->AddChildToWrapBox(InventoryPanelWidget);
	}

	//初始化角色UI
	CharacterPanelWidget = CreateWidget<UCharacterPanelWidget>(GetWorld(), CharacterPanelWidgetClass);
	if (CharacterPanelWidget)
	{
		CharacterPanelWidget->SetVisibility(ESlateVisibility::Hidden);
		CharacterWrapBox->AddChildToWrapBox(CharacterPanelWidget);
	}

	checkf(InventoryPanelWidget, TEXT("InventoryPanelWidget 未初始化"));
	checkf(CharacterPanelWidget, TEXT("ICharacterPanelWidget 未初始化"));
}
