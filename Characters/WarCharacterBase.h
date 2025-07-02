#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Character.h"
#include "War/WarComponents/PersistentSystem/Interface/WarSaveGameInterface.h"
#include "WarCharacterBase.generated.h"

class UWarAbilitySystemComponent;
class UWarAttributeSet;
class UGameplayEffect;
class UAbilitiesConfig;

UCLASS()
class WAR_API AWarCharacterBase : public ACharacter, public IWarSaveGameInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
	FGuid PersistentID = FGuid();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
	FName TableRowID = FName();

	UPROPERTY()
	TObjectPtr<UWarAbilitySystemComponent> WarAbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UWarAttributeSet> WarAttributeSet;

	UPROPERTY(EditDefaultsOnly, Category="Ability")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	UAbilitiesConfig* AbilitiesConfig;

	void InitDefaultAttributes() const;

public:
	AWarCharacterBase();

	virtual void BeginPlay() override;

	FORCEINLINE virtual void SaveActorData(FMemoryWriter& MemoryWriter) const override;
	FORCEINLINE virtual void LoadActorData(FMemoryReader& MemoryReader) override;
	FORCEINLINE virtual FGuid GetPersistentID() const override { return PersistentID; };
	virtual void SetPersistentID(const FGuid& NewID) override;
	FORCEINLINE virtual FName GetTableRowID() const override { return TableRowID; }

	//!IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface
	virtual UWarAttributeSet* GetAttributeSet() const;
};
