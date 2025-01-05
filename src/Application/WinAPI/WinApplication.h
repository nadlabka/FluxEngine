#pragma once
#include <windows.h>

namespace Core
{
    class FluxEngine;
}
namespace Application
{
    class WinWindow;
}


namespace Application
{
    class WinApplication
    {
    public:
        static void Init(Core::FluxEngine* engine, HINSTANCE hInstance, int nCmdShow, int windowWidth, int windowHeight, const std::wstring& title);
        static int Run();
        static WinWindow& GetWindow() { return m_window; }

        static std::wstring GetTitle();
        static void SetTitle(const std::wstring& title);

        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    private:
        static Core::FluxEngine* m_engine;
        static WinWindow m_window;
        static std::wstring m_title;
    };
}