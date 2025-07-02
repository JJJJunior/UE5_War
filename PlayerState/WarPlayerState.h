#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "WarPlayerState.generated.h"

class UWarAbilitySystemComponent;
class UWarAttributeSet;

UCLASS()
class WAR_API AWarPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	TObjectPtr<UWarAbilitySystemComponent> WarAbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UWarAttributeSet> WarAttributeSet;

public:
	AWarPlayerState();
	//~IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface
	virtual UWarAttributeSet* GetAttributeSet() const;
};
