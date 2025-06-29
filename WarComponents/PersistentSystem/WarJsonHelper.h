#pragma once

#include "CoreMinimal.h"

class WAR_API WarJsonHelper
{
public:
	static bool ExtractEquippedInventoryKeys(const TSharedPtr<FJsonObject>& Json,FGuid& OutPlayerID, bool& OutEquipped);
	static bool ExtractInventoryKeys(const TSharedPtr<FJsonObject>& Json, FGuid& OutInstanceID, FGuid& OutPlayerID);
	static bool ExtractInventoryWithTable(const TSharedPtr<FJsonObject>& Json, FGuid& OutPlayerID, FName& OutTableRowID);
	static bool ExtractInventoryOnlyID(const TSharedPtr<FJsonObject>& Json, FGuid& OutInstanceID);
	static bool ExtractInventoryOnlyPlayerID(const TSharedPtr<FJsonObject>& Json, FGuid& OutPlayerID);
};
