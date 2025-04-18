#include "stdafx.h"
#include "KeyboardManager.h"
#include <DebugMacros.h>

void KeyboardManager::InitWinApiMapping()
{
    KeycodeMapping& keyMapping = m_keycodeMapping;

    keyMapping['0'] = eKeycode_0;
    keyMapping['1'] = eKeycode_1;
    keyMapping['2'] = eKeycode_2;
    keyMapping['3'] = eKeycode_3;
    keyMapping['4'] = eKeycode_4;
    keyMapping['5'] = eKeycode_5;
    keyMapping['6'] = eKeycode_6;
    keyMapping['7'] = eKeycode_7;
    keyMapping['8'] = eKeycode_8;
    keyMapping['9'] = eKeycode_9;

    keyMapping['A'] = eKeycode_A;
    keyMapping['B'] = eKeycode_B;
    keyMapping['C'] = eKeycode_C;
    keyMapping['D'] = eKeycode_D;
    keyMapping['E'] = eKeycode_E;
    keyMapping['F'] = eKeycode_F;
    keyMapping['G'] = eKeycode_G;
    keyMapping['H'] = eKeycode_H;
    keyMapping['I'] = eKeycode_I;
    keyMapping['J'] = eKeycode_J;
    keyMapping['K'] = eKeycode_K;
    keyMapping['L'] = eKeycode_L;
    keyMapping['M'] = eKeycode_M;
    keyMapping['N'] = eKeycode_N;
    keyMapping['O'] = eKeycode_O;
    keyMapping['P'] = eKeycode_P;
    keyMapping['Q'] = eKeycode_Q;
    keyMapping['R'] = eKeycode_R;
    keyMapping['S'] = eKeycode_S;
    keyMapping['T'] = eKeycode_T;
    keyMapping['U'] = eKeycode_U;
    keyMapping['V'] = eKeycode_V;
    keyMapping['W'] = eKeycode_W;
    keyMapping['X'] = eKeycode_X;
    keyMapping['Y'] = eKeycode_Y;
    keyMapping['Z'] = eKeycode_Z;

    keyMapping[VK_SPACE] = eKeycode_Space;
    keyMapping[VK_SHIFT] = eKeycode_LShift;
    keyMapping[VK_MENU] = eKeycode_LAlt;
    keyMapping[VK_CONTROL] = eKeycode_LCtrl;
    keyMapping[VK_TAB] = eKeycode_Tab;
    keyMapping[VK_CAPITAL] = eKeycode_Capslock;
    keyMapping[VK_LWIN] = eKeycode_LWin;

    keyMapping[VK_F1] = eKeycode_F1;
    keyMapping[VK_F2] = eKeycode_F2;
    keyMapping[VK_F3] = eKeycode_F3;
    keyMapping[VK_F4] = eKeycode_F4;
    keyMapping[VK_F5] = eKeycode_F5;
    keyMapping[VK_F6] = eKeycode_F6;
    keyMapping[VK_F7] = eKeycode_F7;
    keyMapping[VK_F8] = eKeycode_F8;
    keyMapping[VK_F9] = eKeycode_F9;
    keyMapping[VK_F10] = eKeycode_F10;
    keyMapping[VK_F11] = eKeycode_F11;
    keyMapping[VK_F12] = eKeycode_F12;
}

EKeyboardKey KeyboardManager::GetPlatformSpecificKeycode(uint16_t key)
{
    ASSERT(m_keycodeMapping.find(key) != m_keycodeMapping.end(), "Keycodde is not present in the mapping for this platform");
    return m_keycodeMapping[key];
}

void KeyboardManager::SetKeyState(EKeyboardKey key, EKeyboardKeyState state)
{
    m_keysStates[key] = state;
}

EKeyboardKeyState KeyboardManager::GetKeyState(EKeyboardKey key) const
{
    return m_keysStates[key];
}
