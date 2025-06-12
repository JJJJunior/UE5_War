#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "War/Types/WarInputActionType.h"
#include "DataAsset_InputConfig.generated.h"


class UInputMappingContext;

UCLASS()
class WAR_API UDataAsset_InputConfig : public UDataAsset
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="InputData")
	TArray<FWarInputActionType> WarInputActionTypes;

public:
	UInputAction* FindNativeInputAction(const FGameplayTag& InputTag) const;
};
