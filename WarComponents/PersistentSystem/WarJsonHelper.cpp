#include "WarJsonHelper.h"


bool WarJsonHelper::ExtractEquippedInventoryKeys(const TSharedPtr<FJsonObject>& Json, FGuid& OutInstanceID, FGuid& OutPlayerID, bool& OutEquipped)
{
	FString InstanceIDStr, PlayerIDStr;
	if (!Json.IsValid()) return false;

	if (!Json->TryGetStringField(TEXT("InstanceID"), InstanceIDStr) ||
		!Json->TryGetStringField(TEXT("PlayerID"), PlayerIDStr) ||
		!Json->HasTypedField<EJson::Boolean>(TEXT("bIsEquipped")) ||
		!Json->TryGetBoolField(TEXT("bIsEquipped"), OutEquipped))
	{
		return false;
	}

	return FGuid::Parse(InstanceIDStr, OutInstanceID) && FGuid::Parse(PlayerIDStr, OutPlayerID);
}

bool WarJsonHelper::ExtractInventoryKeys(const TSharedPtr<FJsonObject>& Json, FGuid& OutInstanceID, FGuid& OutPlayerID)
{
	FString InstanceIDStr, PlayerIDStr;
	if (!Json.IsValid()) return false;

	if (!Json->TryGetStringField(TEXT("InstanceID"), InstanceIDStr) ||
		!Json->TryGetStringField(TEXT("PlayerID"), PlayerIDStr))
	{
		return false;
	}

	return FGuid::Parse(InstanceIDStr, OutInstanceID) && FGuid::Parse(PlayerIDStr, OutPlayerID);
}

bool WarJsonHelper::ExtractInventoryWithTable(const TSharedPtr<FJsonObject>& Json, FGuid& OutPlayerID, FName& OutTableRowID)
{
	FString TableRowIDStr, PlayerIDStr;
	if (!Json.IsValid()) return false;
	if (!Json->TryGetStringField(TEXT("TableRowID"), TableRowIDStr) ||
		!Json->TryGetStringField(TEXT("PlayerID"), PlayerIDStr))
	{
		return false;
	}
	if (!FGuid::Parse(PlayerIDStr, OutPlayerID))
	{
		return false;
	}

	OutTableRowID = TableRowIDStr == TEXT("None") ? FName() : FName(*TableRowIDStr);

	return true;
}

bool WarJsonHelper::ExtractInventoryOnlyID(const TSharedPtr<FJsonObject>& Json, FGuid& OutInstanceID)
{
	FString InstanceIDStr;
	if (!Json.IsValid()) return false;

	if (!Json->TryGetStringField(TEXT("InstanceID"), InstanceIDStr))
	{
		return false;
	}

	return FGuid::Parse(InstanceIDStr, OutInstanceID);
}

bool WarJsonHelper::ExtractInventoryOnlyPlayerID(const TSharedPtr<FJsonObject>& Json, FGuid& OutPlayerID)
{
	FString PlayerIDStr;
	if (!Json.IsValid()) return false;

	if (!Json->TryGetStringField(TEXT("PlayerID"), PlayerIDStr))
	{
		return false;
	}

	return FGuid::Parse(PlayerIDStr, OutPlayerID);
}
