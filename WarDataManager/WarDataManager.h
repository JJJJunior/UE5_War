// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SQLiteDatabase.h"
#include "War/WarComponents/InventorySystem/DynamicData/InventoryInstanceData.h"
#include "WarDataManager.generated.h"


UCLASS()
class WAR_API UWarDataManager : public UObject
{
	GENERATED_BODY()
	FSQLiteDatabase Database;

public:
	bool OpenDatabase();
	bool CreateTables();
	void CloseDatabase();
	bool BeginTransaction();
	bool CommitTransaction();
	bool RollbackTransaction();
	bool InsertInventoryData(const FString& InstanceID, const FString& TableRowID, const int32& InventoryType, const FString& ExtraDataType);
	TArray<FInventoryInstanceData> GetAllInventoryItems();
	virtual void BeginDestroy() override;
};
