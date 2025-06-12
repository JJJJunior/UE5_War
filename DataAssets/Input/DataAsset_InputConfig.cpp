#include "DataAsset_InputConfig.h"
#include "GameplayTags.h"

UInputAction* UDataAsset_InputConfig::FindNativeInputAction(const FGameplayTag& InputTag) const
{
	if (!WarInputActionTypes.IsEmpty())
	{
		for (const FWarInputActionType& InputActionConfig : WarInputActionTypes)
		{
			if (InputActionConfig.IsValid() && InputActionConfig.GameplayTag == InputTag)
			{
				return InputActionConfig.InputAction;
			}
		}
	}
	return nullptr;
}
