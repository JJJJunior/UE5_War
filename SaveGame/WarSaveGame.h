#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WarSaveGame.generated.h"


struct FWarSaveGameData;

UCLASS()
class WAR_API UWarSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FWarSaveGameData> GameSavedActors;
};
