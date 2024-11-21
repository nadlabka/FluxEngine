#include <stdafx.h>
#include "WinApplication.h"
#include <FluxEngine.h>
#include "WinWindow.h"

Application::WinWindow Application::WinApplication::m_window;
std::wstring Application::WinApplication::m_title;

int Application::WinApplication::Run(Core::FluxEngine* engine, HINSTANCE hInstance, int nCmdShow, int windowWidth, int windowHeight, const std::wstring& title)
{
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    engine->ParseCommandLineArgs(argv, argc);
    LocalFree(argv);

    m_window = Application::WinWindow(WindowProc, hInstance, nCmdShow, windowWidth, windowHeight, title, engine);

    engine->Init();

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        engine->Update();
        engine->Render();
    }

    engine->Destroy();

    return static_cast<char>(msg.wParam);
}

std::wstring Application::WinApplication::GetTitle()
{
    return m_title;
}

void Application::WinApplication::SetTitle(const std::wstring& title)
{
    m_title = title;
    m_window.SetTitle(title);
}

LRESULT Application::WinApplication::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Core::FluxEngine* engine = reinterpret_cast<Core::FluxEngine*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
        // Save the DXSample* passed in to CreateWindow.
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return 0;

    case WM_KEYDOWN:
        if (engine)
        {
            //InputManager::GetInstance();
            //engine->OnKeyDown(static_cast<UINT8>(wParam));
        }
        return 0;

    case WM_KEYUP:
        if (engine)
        {
            //engine->OnKeyUp(static_cast<UINT8>(wParam));
        }
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(m_window.GetHwnd(), &ps);

        EndPaint(m_window.GetHwnd(), &ps);
        return 0;
    }     

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
