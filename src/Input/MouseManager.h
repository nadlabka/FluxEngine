#pragma once
#include <unordered_map>
#include "MouseKey.h"
#include <array>

struct MousePosition
{
	int xPos;
	int yPos;
};

class MouseManager
{
public:
    static MouseManager& GetInstance()
    {
        static MouseManager instance;
        return instance;
    }

    MouseManager(const MouseManager& arg) = delete;
    MouseManager& operator=(const MouseManager& arg) = delete;

    void InitWinApiMapping();

    void SetButtonState(EMouseButton button, EMouseButtonState state);
    EMouseButtonState GetButtonState(EMouseButton button);

private:
    MouseManager() {}

    std::array<EMouseButtonState, EMouseButton::eMouseButton_NumButtons> m_buttonStates = {};
};