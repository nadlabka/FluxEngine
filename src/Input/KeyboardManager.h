#pragma once
#include <unordered_map>
#include "KeyboardKey.h"
#include <array>
#include <windows.h>
#include <queue>

class KeyboardManager
{
public:
    using KeycodeMapping = std::unordered_map<WORD, EKeyboardKey>;

    static KeyboardManager& GetInstance()
    {
        static KeyboardManager instance;
        return instance;
    }

	KeyboardManager(const KeyboardManager& arg) = delete;
	KeyboardManager& operator=(const KeyboardManager& arg) = delete; 

    void InitWinApiMapping();

    EKeyboardKey GetPlatformSpecificKeycode(uint16_t key);
    
    void SetKeyState(EKeyboardKey key, EKeyboardKeyState state);
    EKeyboardKeyState GetKeyState(EKeyboardKey key) const;

private:
    KeyboardManager() {}

    KeycodeMapping m_keycodeMapping;
    std::array<EKeyboardKeyState, EKeyboardKey::eKeycode_NumKeycodes> m_keysStates = {};
};