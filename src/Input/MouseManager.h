#pragma once
#include <unordered_map>
#include "MouseKey.h"
#include <array>
#include <queue>


class MouseManager
{
public:
    using MouseButtonMapping = std::unordered_map<WORD, EMouseButton>;

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

    void UpdateMousePosition(const Vector2& position);
    const Vector2& GetMouseDelta() const;

private:
    MouseManager() {}

    MouseButtonMapping m_MouseButtonMapping;
    std::array<EMouseButtonState, EMouseButton::eMouseButton_NumButtons> m_buttonStates = {};
    Vector2 m_position = { 0, 0 };
    Vector2 m_deltaPos = { 0, 0 }; //add wheel later
};