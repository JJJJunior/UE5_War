#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "War/WarComponents/PersistentSystem/Interface/WarSaveGameInterface.h"
#include "WarCharacterBase.generated.h"


UCLASS()
class WAR_API AWarCharacterBase : public ACharacter, public IWarSaveGameInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
	FGuid PersistentID = FGuid();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
	FName TableRowID = FName();

public:
	AWarCharacterBase();

	virtual void BeginPlay() override;

	FORCEINLINE virtual void SaveActorData(FMemoryWriter& MemoryWriter) const override;
	FORCEINLINE virtual void LoadActorData(FMemoryReader& MemoryReader) override;
	FORCEINLINE virtual FGuid GetPersistentID() const override { return PersistentID; };
	virtual void SetPersistentID(const FGuid& NewID) override;
	FORCEINLINE virtual FName GetTableRowID() const override { return TableRowID; }
};
