#include "RootPanelWidget.h"
#include "Components/WrapBox.h"
#include "DataManager/ConfigData/GameConfigData.h"
#include "GameInstance/WarGameInstanceSubSystem.h"
#include "Kismet/GameplayStatics.h"
#include "War/WarComponents/InventorySystem/UI/InventoryPanel/InventoryPanelWidget.h"


void URootPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UWarGameInstanceSubSystem* Subsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem) return;
	checkf(Subsystem->GetCachedGameConfigData()->InventoryPanelWidgetClass, TEXT("InventoryPanelWidgetClass 没有配置"));
	//初始化UI背包
	InventoryPanelWidget = CreateWidget<UInventoryPanelWidget>(GetWorld(), Subsystem->GetCachedGameConfigData()->InventoryPanelWidgetClass);
	if (InventoryPanelWidget)
	{
		InventoryPanelWidget->SetVisibility(ESlateVisibility::Hidden);
		InventoryWrapBox->AddChildToWrapBox(InventoryPanelWidget);
	}
	checkf(InventoryPanelWidget, TEXT("InventoryPanelWidget 未初始化"));
}
