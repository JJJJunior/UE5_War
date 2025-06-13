#include "RootPanelWidget.h"
#include "Components/WrapBox.h"
#include "War/WarComponents/InventorySystem/UI/CharacterPanel/CharacterPanelWidget.h"
#include "War/WarComponents/InventorySystem/UI/InventoryPanel/InventoryPanelWidget.h"
#include "War/WarComponents/InventorySystem/UI/QuickPanel/QuickPanelWidget.h"


void URootPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	checkf(InventoryPanelWidgetClass, TEXT("InventoryPanelWidgetClass 没有配置"));
	checkf(CharacterPanelWidgetClass, TEXT("CharacterPanelWidgetClass 没有配置"));
	checkf(QuickPanelWidgetClass, TEXT("QuickPanelWidgetClass 没有配置"));


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

	//初始化快捷UI
	QuickPanelWidget = CreateWidget<UQuickPanelWidget>(GetWorld(), QuickPanelWidgetClass);
	if (QuickPanelWidget)
	{
		QuickPanelWidget->SetVisibility(ESlateVisibility::Visible);
		QuickWrapBox->AddChildToWrapBox(QuickPanelWidget);
	}


	checkf(InventoryPanelWidget, TEXT("InventoryPanelWidget 未初始化"));
	checkf(CharacterPanelWidget, TEXT("CharacterPanelWidget 未初始化"));
	checkf(QuickPanelWidget, TEXT("QuickPanelWidget 未初始化"));
}
