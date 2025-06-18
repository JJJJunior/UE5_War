#pragma once

#define print(Format, ...) UE_LOG(LogTemp, Warning,Format TEXT(" [%s:%d]"),##__VA_ARGS__,TEXT(__FILE__), __LINE__)
