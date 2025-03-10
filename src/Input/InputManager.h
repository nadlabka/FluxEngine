#pragma once
#include <unordered_map>
#include <functional>
#include "KeyboardKey.h"
#include "MouseKey.h"
#include "DataStructures/SolidVector.h"

class InputManager
{
public:
	using InputCallback = std::function<void()>;
	using CallbackID = SolidVector<InputCallback>::ID;

	static InputManager& GetInstance()
	{
		static InputManager instance;
		return instance;
	}

	InputManager(const InputManager& arg) = delete;
	InputManager& operator=(const InputManager& arg) = delete;

	void Initialize();
	void Update();

	CallbackID RegisterKeyCallback(EKeyboardKey key, EKeyboardKeyState state, InputCallback callback);

	CallbackID RegisterMouseButtonCallback(EMouseButton button, EMouseButtonState state, InputCallback callback);

	CallbackID RegisterMouseMoveCallback(InputCallback callback);

private:
	InputManager() {}
	std::array<std::array<SolidVector<InputCallback>, eKeyboardKeyState_NumStates>, eKeycode_NumKeycodes> m_keyCallbacks;
	std::array<std::array<SolidVector<InputCallback>, eMouseButtonState_NumStates>, eMouseButton_NumButtons> m_mouseButtonCallbacks;
	SolidVector<InputCallback> m_mouseMoveCallbacks;
};