#pragma once
#include <windows.h>

class FluxEngine;
class WinWindow;

class WinApplication
{
public:
    static int Run(FluxEngine* engine, HINSTANCE hInstance, int nCmdShow, int windowWidth, int windowHeight, const std::wstring& title);
    static WinWindow& GetWindow() { return m_window; }

    static std::wstring GetTitle();
    static void SetTitle(const std::wstring& title);

    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static WinWindow m_window;
    static std::wstring m_title;
};