#pragma once

#include "CoreMinimal.h"
#include "DataManager/DynamicData/InventoryData.h"
#include "GameFramework/SaveGame.h"
#include "WarSaveGame.generated.h"


UCLASS()
class WAR_API UWarSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FWarSaveGameData> GameSavedActors;
};
