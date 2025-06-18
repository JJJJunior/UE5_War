// Fill out your copyright notice in the Description page of Project Settings.


#include "WarDataManager.h"


void UWarDataManager::BeginDestroy()
{
	UObject::BeginDestroy();
	if (Database.IsValid())
	{
		Database.Close();
	}
}


bool UWarDataManager::OpenDatabase()
{
	FString DBPath = FPaths::ProjectContentDir() + "WarData/war.db";
	UE_LOG(LogTemp, Warning, TEXT("OpenDatabase %s"), *DBPath);
	if (!Database.Open(*DBPath, ESQLiteDatabaseOpenMode::ReadWriteCreate))
	{
		UE_LOG(LogTemp, Error, TEXT("打开数据库失败"));
		return false;
	}
	return CreateTables();
}

bool UWarDataManager::CreateTables()
{
	const FString SQL = TEXT(R"(
	-- 主物品表（所有物品类型的公共字段）
CREATE TABLE IF NOT EXISTS InventoryItems (
    Id INTEGER PRIMARY KEY AUTOINCREMENT,
    InstanceID TEXT NOT NULL UNIQUE,          -- 物品实例唯一标识
    TableRowID TEXT NOT NULL,                 -- 数据表行ID（关联配置数据）
    InventoryType INTEGER NOT NULL,           -- 物品类型枚举值（0=装备,1=消耗品,2=任务物品等）
    CreatedTime TEXT DEFAULT CURRENT_TIMESTAMP,-- 创建时间（ISO8601格式）
    LastUpdated TEXT DEFAULT CURRENT_TIMESTAMP,-- 最后更新时间
    ExtraData TEXT,                           -- 预留JSON扩展字段
    -- 约束
    CHECK (InventoryType BETWEEN 0 AND 2),    -- 限制有效类型范围
    CHECK (length(InstanceID) = 36)           -- GUID格式验证

-- 装备数据子表
CREATE TABLE IF NOT EXISTS WeaponInstanceData (
    InstanceID TEXT PRIMARY KEY,
    Durability REAL NOT NULL DEFAULT 100.0     -- 耐久度（默认满耐久）
        CHECK (Durability BETWEEN 0.0 AND 100.0),
    AttackPower REAL NOT NULL DEFAULT 10.0     -- 攻击力
        CHECK (AttackPower >= 0),
    UpgradeLevel INTEGER DEFAULT 1,            -- 强化等级
    -- 外键关联主表
    FOREIGN KEY (InstanceID) 
        REFERENCES InventoryItems(InstanceID)
        ON DELETE CASCADE
)

-- 消耗品数据子表
CREATE TABLE IF NOT EXISTS ConsumableInstanceData (
    InstanceID TEXT PRIMARY KEY,
    StackCount INTEGER NOT NULL DEFAULT 1      -- 堆叠数量
        CHECK (StackCount > 0),
    IsQuestRelated INTEGER DEFAULT 0,          -- 是否任务相关（0/1布尔值）
    ExpirationTime TEXT,                       -- 过期时间（ISO8601）
    -- 外键关联
    FOREIGN KEY (InstanceID) 
        REFERENCES InventoryItems(InstanceID)
        ON DELETE CASCADE
)

-- 任务物品数据子表
CREATE TABLE IF NOT EXISTS QuestItemInstanceData (
    InstanceID TEXT PRIMARY KEY,
    QuestID TEXT NOT NULL,                     -- 关联的任务ID
    IsCollected INTEGER DEFAULT 0,             -- 是否已收集（0/1布尔值）
    ProgressData TEXT,                         -- 任务进度JSON存储
    -- 外键关联
    FOREIGN KEY (InstanceID) 
        REFERENCES InventoryItems(InstanceID)
        ON DELETE CASCADE,
    -- 复合唯一约束
    UNIQUE (InstanceID, QuestID);
)");

	if (!Database.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("数据库尚未打开"));
		return false;
	}

	if (!Database.Execute(*SQL))
	{
		FString ErrorMessage = Database.GetLastError();
		UE_LOG(LogTemp, Error, TEXT("创建表失败: %s"), *ErrorMessage);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("创建表成功"));
	return true;
}

void UWarDataManager::CloseDatabase()
{
	Database.Close();
}

bool UWarDataManager::BeginTransaction()
{
	return Database.Execute(TEXT("Begin Transaction"));
}

bool UWarDataManager::CommitTransaction()
{
	return Database.Execute(TEXT("Commit"));
}

bool UWarDataManager::RollbackTransaction()
{
	return Database.Execute(TEXT("Rollback"));
}

bool UWarDataManager::InsertInventoryData(const FString& InstanceID, const FString& TableRowID, const int32& InventoryType, const FString& ExtraDataType)
{
	const FString SQL = TEXT("INSERT INTO InventoryItems (InstanceID, TableRowID, InventoryType, ExtraDataType) VALUES (?, ?, ?, ?);");
	FSQLitePreparedStatement Statement = Database.PrepareStatement(*SQL);
	if (!Statement.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("InsertInventoryData"));

		return false;
	}

	if (!Statement.SetBindingValueByIndex(1, InstanceID))
	{
		UE_LOG(LogTemp, Error, TEXT("InsertInventoryData"));
		return false;
	}

	if (!Statement.SetBindingValueByIndex(2, TableRowID))
	{
		UE_LOG(LogTemp, Error, TEXT("InsertInventoryData"));
		return false;
	}

	if (!Statement.SetBindingValueByIndex(3, InventoryType))
	{
		UE_LOG(LogTemp, Error, TEXT("InsertInventoryData"));
		return false;
	}
	if (!Statement.SetBindingValueByIndex(4, ExtraDataType))
	{
		UE_LOG(LogTemp, Error, TEXT("InsertInventoryData"));
		return false;
	}

	if (!Statement.Execute())
	{
		UE_LOG(LogTemp, Error, TEXT("InsertInventoryData"));
		return false;
	}
	return true;
}

TArray<FInventoryInstanceData> UWarDataManager::GetAllInventoryItems()
{
	TArray<FInventoryInstanceData> InventoryItems;

	const FString SQL = TEXT("SELECT InstanceID,TableRowID,InventoryType FROM InventoryItems");
	FSQLitePreparedStatement Statement = Database.PrepareStatement(*SQL);

	while (Statement.Step() == ESQLitePreparedStatementStepResult::Row)
	{
		FInventoryInstanceData InventoryItem;

		if (!Statement.GetColumnValueByIndex(0, InventoryItem.InstanceID)) continue;
		if (!Statement.GetColumnValueByIndex(1, InventoryItem.TableRowID))continue;
		if (!Statement.GetColumnValueByIndex(3, InventoryItem.InventoryType))continue;

		InventoryItems.Add(InventoryItem);
	}
	return InventoryItems;
}
