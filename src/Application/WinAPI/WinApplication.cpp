#include <stdafx.h>
#include "WinApplication.h"
#include <FluxEngine.h>
#include "WinWindow.h"

WinWindow WinApplication::m_window;
std::wstring WinApplication::m_title;

int WinApplication::Run(FluxEngine* engine, HINSTANCE hInstance, int nCmdShow, int windowWidth, int windowHeight, const std::wstring& title)
{
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    engine->ParseCommandLineArgs(argv, argc);
    LocalFree(argv);

    m_window = WinWindow(WindowProc, hInstance, nCmdShow, windowWidth, windowHeight, title, engine);

    engine->OnInit();

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        //engine->Update();
        //engine->Render();
    }

    engine->OnDestroy();

    return static_cast<char>(msg.wParam);
}

std::wstring WinApplication::GetTitle()
{
    return m_title;
}

void WinApplication::SetTitle(const std::wstring& title)
{
    m_title = title;
    m_window.SetTitle(title);
}

LRESULT WinApplication::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    FluxEngine* engine = reinterpret_cast<FluxEngine*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

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
        if (engine)
        {
            engine->OnUpdate();
            engine->OnRender();
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
