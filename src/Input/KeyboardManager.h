#pragma once
#include <unordered_map>
#include "KeyboardKey.h"
#include <array>
#include <windows.h>

using KeycodeMapping = std::unordered_map<WORD, EKeyboardKey>;

class KeyboardManager
{
public:
    static KeyboardManager& GetInstance()
    {
        static KeyboardManager instance;
        return instance;
    }

	KeyboardManager(const KeyboardManager& arg) = delete;
	KeyboardManager& operator=(const KeyboardManager& arg) = delete; 

    void InitWinApiMapping();
    
    void SetKeyState(EKeyboardKey key, EKeyboardKeyState state);
    EKeyboardKeyState GetKeyState(EKeyboardKey key) const;

private:
    KeyboardManager() {}

    KeycodeMapping m_KeycodeMapping;
    std::array<EKeyboardKeyState, EKeyboardKey::eKeycode_NumKeycodes> m_keysStates = {};
};