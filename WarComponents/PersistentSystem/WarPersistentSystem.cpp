#include "WarPersistentSystem.h"
#include "EngineUtils.h"
#include "War/GameManager/SaveGame/WarSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Tools/MyLog.h"
#include "War/GameManager/SaveGame/Interface/WarSaveGameInterface.h"


UWarPersistentSystem::UWarPersistentSystem()
{
}


void UWarPersistentSystem::SaveGame()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UWarSaveGame* SaveGameInstance = Cast<UWarSaveGame>(UGameplayStatics::CreateSaveGameObject(UWarSaveGame::StaticClass()));

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (IWarSaveGameInterface* SaveInterface = Cast<IWarSaveGameInterface>(Actor))
		{
			//存档数据构造
			FWarSaveGameData ActorData;
			ActorData.ActorGuid = SaveInterface->GetSaveID();
			ActorData.ActorTransform = Actor->GetActorTransform();
			FMemoryWriter MemoryWriter(ActorData.ActorData, true);
			SaveInterface->SaveActorData(MemoryWriter);

			//存实例
			SaveGameInstance->GameSavedActors.Add(ActorData);
		}
	}
	print(TEXT("游戏已保存------------- "));
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("MySaveSlot"), 0);
}

void UWarPersistentSystem::LoadGame()
{
	UWarSaveGame* LoadGameInstance = Cast<UWarSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("MySaveSlot"), 0));
	if (!LoadGameInstance) return;

	UWorld* World = GetWorld();
	if (!World) return;

	for (const FWarSaveGameData& ActorData : LoadGameInstance->GameSavedActors)
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;

			if (IWarSaveGameInterface* SaveInterface = Cast<IWarSaveGameInterface>(Actor))
			{
				if (SaveInterface->GetSaveID() == ActorData.ActorGuid)
				{
					// 还原 Transform
					Actor->SetActorTransform(ActorData.ActorTransform);
					// 还原状态数据
					FMemoryReader MemoryReader(ActorData.ActorData, true);
					SaveInterface->LoadActorData(MemoryReader);
					break;
				}
			}
		}
	}
	print(TEXT("游戏读取完毕-------------"));
}
