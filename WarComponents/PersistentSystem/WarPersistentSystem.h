#pragma once

#include "CoreMinimal.h"
#include "WarPersistentSystem.generated.h"


class UWarDataManager;


USTRUCT(Blueprintable)
struct FWarSaveGameData
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid ActorGuid;
	UPROPERTY()
	FTransform ActorTransform;
	UPROPERTY()
	TArray<uint8> ActorData; //序列化
};


UCLASS()
class WAR_API UWarPersistentSystem : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FWarSaveGameData> GameSavedActors;

public:
	UWarPersistentSystem();
	UFUNCTION(BlueprintCallable)
	void SaveGame();
	UFUNCTION(BlueprintCallable)
	void LoadGame();
};
