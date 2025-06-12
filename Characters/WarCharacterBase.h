#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WarCharacterBase.generated.h"


UCLASS()
class WAR_API AWarCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AWarCharacterBase();
	virtual void BeginPlay() override;
};
