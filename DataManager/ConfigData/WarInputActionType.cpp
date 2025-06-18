#include "WarInputActionType.h"

bool FWarInputActionType::IsValid() const
{
	return InputAction && GameplayTag.IsValid();
}
