#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "War/GameManager/SaveGame/Interface/WarSaveGameInterface.h"
#include "WarCharacterBase.generated.h"


UCLASS()
class WAR_API AWarCharacterBase : public ACharacter, public IWarSaveGameInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	FGuid PersistentActorID;

public:
	AWarCharacterBase();

	virtual void BeginPlay() override;

	virtual void SaveActorData(FMemoryWriter& MemoryWriter) const override;
	virtual void LoadActorData(FMemoryReader& MemoryReader) const override;
	virtual FGuid GetSaveID() const override;
};
