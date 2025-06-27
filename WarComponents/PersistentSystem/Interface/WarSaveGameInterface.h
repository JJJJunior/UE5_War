#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WarSaveGameInterface.generated.h"


UINTERFACE(MinimalAPI, Blueprintable)
class UWarSaveGameInterface : public UInterface
{
	GENERATED_BODY()
};


class WAR_API IWarSaveGameInterface
{
	GENERATED_BODY()

public:
	virtual void SaveActorData(FMemoryWriter& MemoryWriter) const = 0;
	virtual void LoadActorData(FMemoryReader& MemoryReader) const = 0;
	virtual FGuid GetPersistentID() const = 0;
	virtual void SetPersistentID(const FGuid& NewID) = 0;
	virtual FName GetTableRowID() const = 0;
};
