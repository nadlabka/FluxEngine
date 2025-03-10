#include "stdafx.h"
#include "MouseManager.h"

void MouseManager::InitWinApiMapping()
{
	MouseButtonMapping& buttonMapping = m_MouseButtonMapping;

	buttonMapping[VK_LBUTTON] = eMouseButton_Left;
	buttonMapping[VK_RBUTTON] = eMouseButton_Right;
	buttonMapping[VK_MBUTTON] = eMouseButton_Middle;
}

void MouseManager::SetButtonState(EMouseButton button, EMouseButtonState state)
{
	m_buttonStates[button] = state;
}

EMouseButtonState MouseManager::GetButtonState(EMouseButton button)
{
	return m_buttonStates[button];
}

void MouseManager::UpdateMousePosition(const Vector2& position)
{
	m_deltaPos = position - m_position;
	m_position = position;
}

const Vector2& MouseManager::GetMouseDelta() const
{
	return m_deltaPos;
}
