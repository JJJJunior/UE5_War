#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "war/SaveGame/Interface/WarSaveGameInterface.h"
#include "WarCharacterBase.generated.h"


UCLASS()
class WAR_API AWarCharacterBase : public ACharacter, public IWarSaveGameInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
	FGuid PersistentID = FGuid();

public:
	AWarCharacterBase();

	virtual void BeginPlay() override;

	virtual void SaveActorData(FMemoryWriter& MemoryWriter) const override;
	virtual void LoadActorData(FMemoryReader& MemoryReader) const override;
	virtual FGuid GetPersistentID() const override;
	virtual void SetPersistentID(const FGuid& NewID) override;
};
