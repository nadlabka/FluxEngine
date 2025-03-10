#include "stdafx.h"
#include "InputManager.h"
#include "MouseManager.h"
#include "KeyboardManager.h"

void InputManager::Initialize()
{
	MouseManager::GetInstance().InitWinApiMapping();
	KeyboardManager::GetInstance().InitWinApiMapping();
}

void InputManager::Update()
{
    for (uint16_t key = 0; key < eKeycode_NumKeycodes; ++key) 
    {
        EKeyboardKey k = static_cast<EKeyboardKey>(key);
        EKeyboardKeyState state = KeyboardManager::GetInstance().GetKeyState(k);
        const auto& callbacks = m_keyCallbacks[key][static_cast<size_t>(state)];
        for (uint32_t i = 0; i < callbacks.size(); ++i) 
        {
            callbacks.data()[i]();
        }
    }

    for (uint8_t btn = 0; btn < eMouseButton_NumButtons; ++btn) 
    {
        EMouseButton b = static_cast<EMouseButton>(btn);
        EMouseButtonState state = MouseManager::GetInstance().GetButtonState(b);
        const auto& callbacks = m_mouseButtonCallbacks[btn][static_cast<size_t>(state)];
        for (uint32_t i = 0; i < callbacks.size(); ++i) 
        {
            callbacks.data()[i]();
        }
    }

    if (MouseManager::GetInstance().GetMouseDelta().x != 0 || MouseManager::GetInstance().GetMouseDelta().y != 0)
    {
        for (uint32_t i = 0; i < m_mouseMoveCallbacks.size(); ++i) 
        {
            m_mouseMoveCallbacks.data()[i]();
        }
    }
}

InputManager::CallbackID InputManager::RegisterKeyCallback(EKeyboardKey key, EKeyboardKeyState state, InputCallback callback)
{
    return m_keyCallbacks[key][static_cast<size_t>(state)].insert(callback);
}

InputManager::CallbackID InputManager::RegisterMouseButtonCallback(EMouseButton button, EMouseButtonState state, InputCallback callback)
{
    return m_mouseButtonCallbacks[button][static_cast<size_t>(state)].insert(callback);
}

InputManager::CallbackID InputManager::RegisterMouseMoveCallback(InputCallback callback)
{
    return m_mouseMoveCallbacks.insert(callback);
}
