#include "WarGameInstanceSubSystem.h"
#include "EngineUtils.h"
#include "War/DataManager/ConfigData/GameConfigData.h"
#include "War/WarComponents/PersistentSystem/WarPersistentSystem.h"
#include "Tools/MyLog.h"
#include "War/Characters/Hero/WarHeroCharacter.h"

void UWarGameInstanceSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 创建软引用
	TSoftObjectPtr<UGameConfigData> SoftConfigData = TSoftObjectPtr<UGameConfigData>(FSoftObjectPath(TEXT("/Game/ThirdPerson/DA_GameConfig.DA_GameConfig")));
	// 同步加载资源
	GameConfigData = SoftConfigData.LoadSynchronous();
	checkf(GameConfigData, TEXT("GameConfigData is NULL"))

	WarInventoryDataTable = GameConfigData->WarInventoryDataTable.LoadSynchronous();
	WarAbilityDataTable = GameConfigData->WarAbilityDataTable.LoadSynchronous();
	if (!IsValid(WarInventoryDataTable))
	{
		print(TEXT("WarInventoryDataTable 空指针"))
		return;
	}
	if (!IsValid(WarAbilityDataTable))
	{
		print(TEXT("WarAbilityDataTable 空指针"))
		return;
	}

	WarPersistentSystem = NewObject<UWarPersistentSystem>(this);
	if (!IsValid(WarPersistentSystem))
	{
		UE_LOG(LogTemp, Error, TEXT("WarPersistentSystem is NULL"));
		return;
	}
}


void UWarGameInstanceSubSystem::Deinitialize()
{
	Super::Deinitialize();
}


TSoftObjectPtr<UMaterialInstance> UWarGameInstanceSubSystem::GetOutLineMaterial() const
{
	return GameConfigData->OutLineMaterial;
}

const FWarInventoryRow* UWarGameInstanceSubSystem::FindInventoryRow(const UObject* WorldContextObject, const FName& TableRowID)
{
	if (!WorldContextObject)
	{
		print(TEXT("WorldContextObject 不存在"))
		return nullptr;
	}
	UWarGameInstanceSubSystem* Subsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem || !Subsystem->WarInventoryDataTable)
	{
		print(TEXT("Subsystem or WarInventoryDataTable 不存在"))
		return nullptr;
	}
	FWarInventoryRow* FindItemRow = Subsystem->WarInventoryDataTable->FindRow<FWarInventoryRow>(TableRowID, "FindInventoryRow");
	if (!FindItemRow)
	{
		print(TEXT("FindItemRow 不存在"))
		return nullptr;
	}
	return FindItemRow;
}

TWeakObjectPtr<UGameConfigData> UWarGameInstanceSubSystem::GetGameConfigData(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		print(TEXT("WorldContextObject 不存在"))
		return nullptr;
	}
	UWarGameInstanceSubSystem* Subsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem || !Subsystem->WarInventoryDataTable)
	{
		print(TEXT("Subsystem or WarInventoryDataTable 不存在"))
		return nullptr;
	}

	return Subsystem->GameConfigData;
}

FString UWarGameInstanceSubSystem::GetStaticPlayerID(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		print(TEXT("WorldContextObject 不存在"))
		return FString();
	}
	UWarGameInstanceSubSystem* Subsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UWarGameInstanceSubSystem>();
	if (!Subsystem || !Subsystem->WarInventoryDataTable)
	{
		print(TEXT("Subsystem or WarInventoryDataTable 不存在"))
		return FString();
	}
	return Subsystem->GameConfigData->StaticPlayerID;
}

AWarHeroCharacter* UWarGameInstanceSubSystem::FindCharacterByPersistentID(const UObject* WorldContextObject, const FGuid& PersistentID)
{
	if (!WorldContextObject) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	for (TActorIterator<AWarHeroCharacter> It(World); It; ++It)
	{
		AWarHeroCharacter* Character = *It;
		if (!Character) continue;

		if (Character->GetPersistentID() == PersistentID)
		{
			return Character;
		}
	}

	return nullptr;
}
