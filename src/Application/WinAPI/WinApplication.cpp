#include <stdafx.h>
#include "WinApplication.h"
#include <FluxEngine.h>
#include "WinWindow.h"
#include <ECS/Entity.h>
#include <ECS/Components/InstancedStaticMesh.h>
#include <ECS/Components/Transform.h>
#include <ECS/Components/HierarchyRelationship.h>
#include <CubeMeshLoader.h>

Application::WinWindow Application::WinApplication::m_window;
std::wstring Application::WinApplication::m_title;
Core::FluxEngine* Application::WinApplication::m_engine;

void Application::WinApplication::Init(Core::FluxEngine* engine, HINSTANCE hInstance, int nCmdShow, int windowWidth, int windowHeight, const std::wstring& title)
{
    m_engine = engine;

    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    m_engine->ParseCommandLineArgs(argv, argc);
    LocalFree(argv);

    m_window = Application::WinWindow(WindowProc, hInstance, nCmdShow, windowWidth, windowHeight, title, engine);

    engine->Init();

    //custom client entity-related init logic is currently executed here
    auto& entityManager = Core::EntitiesPool::GetInstance();
    auto cubeEntity = entityManager.CreateEntity();
    auto& cubeMeshComponent = cubeEntity.AddComponent<Components::InstancedStaticMesh>(LoadCubeMesh());

    auto& cubeTransformComponent = cubeEntity.AddComponent<Components::Transform>();
    cubeTransformComponent.position = { 0, 0, 0.5 };
    cubeTransformComponent.rotationAngles = { 0.0f, 0.0f, 0.0f };
    cubeTransformComponent.scale = { 0.1, 0.1, 0.1 };

    auto& cubeRelationshipComponent = cubeEntity.AddComponent<Components::HierarchyRelationship>();
}

int Application::WinApplication::Run()
{
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        m_engine->Update();
        m_engine->Render();
    }

    m_engine->Destroy();

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
