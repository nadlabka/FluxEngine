#pragma once
#include <cstdint>
#include <Windows.h>

class FluxEngine;

class WinWindow
{
public:
	WinWindow();
	WinWindow(WNDPROC winProc, HINSTANCE hInstance, bool nCmdShow, int width, int height, const std::wstring& title, FluxEngine* engine);
	~WinWindow();

	HWND GetHwnd() const { return m_hwnd; }

	void SetTitle(const std::wstring& title);

	void Resize(int width, int height);

	int GetHeight() const { return m_height; }
	int GetWidth() const { return m_width; }

	float GetAspectRatio() const { return m_aspectRatio; }

	static uint32_t GetTotalWindowsNum() { return m_totalWindowsNum; }

private:
	HWND m_hwnd;

	int m_width;
	int m_height;

	float m_aspectRatio;

	HINSTANCE m_hInstance;
	uint32_t m_windowNum;
	std::wstring m_windowClassName;

	static uint32_t m_totalWindowsNum;
};