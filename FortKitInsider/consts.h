#pragma once

namespace Patterns
{
    constexpr auto GObjects = "48 8B 05 ? ? ? ? 48 8B 0C C8 48 8D 04 D1 EB 06";
}

namespace StringRefs
{
    constexpr auto FNameToString = L"%s %s SetTimer passed a negative or zero time. The associated timer may fail to be created/fire! If using InitialStartDelayVariance, be sure it is smaller than (Time + InitialStartDelay).";
}