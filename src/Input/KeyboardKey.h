#pragma once
#include <cstdint>

enum EKeyboardKey : uint16_t
{
    eKeycode_F1, eKeycode_F2, eKeycode_F3, eKeycode_F4, eKeycode_F5, eKeycode_F6,
    eKeycode_F7, eKeycode_F8, eKeycode_F9, eKeycode_F10, eKeycode_F11, eKeycode_F12,

    eKeycode_1, eKeycode_2, eKeycode_3, eKeycode_4, eKeycode_5, eKeycode_6, eKeycode_7, eKeycode_8, eKeycode_9, eKeycode_0,
    eKeycode_A, eKeycode_B, eKeycode_C, eKeycode_D, eKeycode_E, eKeycode_F, eKeycode_G, eKeycode_H,
    eKeycode_I, eKeycode_J, eKeycode_K, eKeycode_L, eKeycode_M, eKeycode_N, eKeycode_O, eKeycode_P,
    eKeycode_Q, eKeycode_R, eKeycode_S, eKeycode_T, eKeycode_U, eKeycode_V, eKeycode_W, eKeycode_X,
    eKeycode_Y, eKeycode_Z,

    eKeycode_Slash,

    eKeycode_Space, eKeycode_Capslock, eKeycode_Tab,
    eKeycode_LCtrl, eKeycode_RCtrl, eKeycode_LShift, eKeycode_RShift, eKeycode_LAlt, eKeycode_RAlt,
    eKeycode_LWin,

    eKeycode_NumKeycodes,
};

enum EKeyboardKeyState : uint8_t
{
    eKeyboardKeyState_Released = 0,
    eKeyboardKeyState_Pressed,

    eKeyboardKeyState_NumStates,
};