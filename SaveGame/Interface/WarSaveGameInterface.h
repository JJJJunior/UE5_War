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
	virtual FGuid GetPersistentActorID() const = 0;
};
