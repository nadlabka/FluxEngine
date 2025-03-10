#pragma once
#include <cstdint>
#include <unordered_map>

enum EMouseButton : uint8_t
{
    eMouseButton_Left,
    eMouseButton_Right,
    eMouseButton_Middle,

    eMouseButton_NumButtons,
};

enum EMouseButtonState : uint8_t
{
    eMouseButtonState_Released = 0,
    eMouseButtonState_ClickedOnce = 1,
    eMouseButtonState_ClickedTwice = 2,

    eMouseButtonState_NumStates,
};