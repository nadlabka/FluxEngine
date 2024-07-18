#include <stdafx.h>
#include "WinWindow.h"
#include <FluxEngine.h>

uint32_t WinWindow::m_totalWindowsNum = 0;

WinWindow::WinWindow()
{
}

WinWindow::WinWindow(WNDPROC winProc, HINSTANCE hInstance, bool nCmdShow, int width, int height, const std::wstring& title, FluxEngine* engine) :
    m_height(height),
    m_width(width),
    m_aspectRatio(float(width)/height),
    m_hInstance(hInstance),
    m_windowNum(m_totalWindowsNum++),
    m_windowClassName(L"WinWindow" + std::to_wstring(m_windowNum))
{
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = winProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = m_windowClassName.c_str();
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    m_hwnd = CreateWindow(
        windowClass.lpszClassName,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        hInstance,
        engine);
    ShowWindow(m_hwnd, nCmdShow);
}

WinWindow::~WinWindow()
{
    UnregisterClass(m_windowClassName.c_str(), m_hInstance);
}

void WinWindow::SetTitle(const std::wstring& title)
{
    SetWindowText(m_hwnd, title.c_str());
}

void WinWindow::Resize(int width, int height)
{
    m_width = width;
    m_height = height;
    m_aspectRatio = float(width) / height;
}
