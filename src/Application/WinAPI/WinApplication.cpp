#include <stdafx.h>
#include "WinApplication.h"
#include <FluxEngine.h>
#include "WinWindow.h"
#include <ECS/Entity.h>
#include <ECS/Components/InstancedStaticMesh.h>
#include <ECS/Components/Transform.h>
#include <ECS/Components/HierarchyRelationship.h>
#include <CubeMeshLoader.h>
#include <ECS/Components/MaterialParameters.h>
#include "../../Input/InputManager.h"
#include <ECS/Components/Camera.h>
#include "../../Input/MouseManager.h"
#include "../../Input/KeyboardManager.h"

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
    uint32_t meshId = LoadCubeMesh();
    auto& cubeMeshComponent = cubeEntity.AddComponent<Components::InstancedStaticMesh>(meshId);

    auto cubeEntity1 = entityManager.CreateEntity();
    auto& cubeMeshComponent1 = cubeEntity1.AddComponent<Components::InstancedStaticMesh>(meshId);

    auto& cubeTransformComponent = cubeEntity.AddComponent<Components::Transform>();
    cubeTransformComponent.position = { 0, 0, 0.5 };
    cubeTransformComponent.rotationAngles = { 30.0f, 30.0f, 30.0f };
    cubeTransformComponent.scale = { 0.1, 0.1, 0.1 };

    auto& hierarchyComp = cubeEntity.AddComponent<Components::HierarchyRelationship>();
    hierarchyComp.children = 1;
    hierarchyComp.first = cubeEntity1;
    cubeEntity.AddComponent<Components::TransformFlags>();
    cubeEntity.AddComponent<Components::AccumulatedHierarchicalTransformMatrix>();

    auto& staticMesh = Assets::AssetsManager<Assets::StaticMesh>::GetInstance().GetAsset(meshId);
    staticMesh.CreatePerInstanceData(cubeEntity, Assets::MeshPerInstanceData{});
    staticMesh.CreateSubmeshPerInstanceData<MaterialParameters::UnlitDefault>(cubeEntity, 0u, MaterialParameters::UnlitDefault{0u});

    auto& cubeTransformComponent1 = cubeEntity1.AddComponent<Components::Transform>();
    cubeTransformComponent1.position = { 3, 2, 0.5 };
    cubeTransformComponent1.rotationAngles = { 30.0f, 30.0f, 30.0f };
    cubeTransformComponent1.scale = { 0.5, 0.5, 0.5 };

    auto& hierarchyComp1 = cubeEntity1.AddComponent<Components::HierarchyRelationship>();
    hierarchyComp1.parent = cubeEntity;
    cubeEntity1.AddComponent<Components::TransformFlags>();
    cubeEntity1.AddComponent<Components::AccumulatedHierarchicalTransformMatrix>();

    staticMesh.CreatePerInstanceData(cubeEntity1, Assets::MeshPerInstanceData{});
    staticMesh.CreateSubmeshPerInstanceData<MaterialParameters::UnlitDefault>(cubeEntity1, 0u, MaterialParameters::UnlitDefault{ 0u });

    auto camera = entityManager.CreateEntity();
    auto& cameraTransformComponent = camera.AddComponent<Components::Transform>();
    cameraTransformComponent.position = { 0, 0, -10 };
    cameraTransformComponent.rotationAngles = { 0, 0, 0 };
    cameraTransformComponent.scale = { 1, 1, 1 };
    auto& cameraComponent = camera.AddComponent<Components::Camera>();
    cameraComponent.aspectRatio = m_window.GetAspectRatio();
    cameraComponent.nearPlane = 0.01f;
    cameraComponent.farPlane = 1000.0f;
    cameraComponent.fovY = 90.0f;
    cameraComponent.forward.x = std::cos(cameraTransformComponent.rotationAngles.y) * std::cos(cameraTransformComponent.rotationAngles.x);
    cameraComponent.forward.y = std::sin(cameraTransformComponent.rotationAngles.y);
    cameraComponent.forward.z = std::cos(cameraTransformComponent.rotationAngles.y) * std::sin(cameraTransformComponent.rotationAngles.x);
    cameraComponent.forward.Normalize();
    Vector3 globalUp{ 0.0f, 1.0f, 0.0f };
    cameraComponent.right = cameraComponent.forward.Cross(globalUp);
    cameraComponent.right.Normalize();
    cameraComponent.up = cameraComponent.right.Cross(cameraComponent.forward);
    cameraComponent.up.Normalize();
    auto& cameraControlComponent = camera.AddComponent<Components::CameraControl>();
    cameraControlComponent.sensetivity = 0.001f;
    cameraControlComponent.speed = 0.1f;
    cameraControlComponent.isRotating = false;

    InputManager& input = InputManager::GetInstance();

    input.RegisterKeyCallback(eKeycode_W, eKeyboardKeyState_Pressed,
        [camera]()
        {
            auto& entityManager = Core::EntitiesPool::GetInstance();
            auto& cameraComponent = entityManager.GetRegistry().get<Components::Camera>(camera);
            auto& transformComponent = entityManager.GetRegistry().get<Components::Transform>(camera);
            auto& controlComponent = entityManager.GetRegistry().get<Components::CameraControl>(camera);
                
            Vector3 move = cameraComponent.forward * controlComponent.speed;
            transformComponent.position += move;
        });

    input.RegisterKeyCallback(eKeycode_S, eKeyboardKeyState_Pressed,
        [camera]()
        {
            auto& entityManager = Core::EntitiesPool::GetInstance();
            auto& cameraComponent = entityManager.GetRegistry().get<Components::Camera>(camera);
            auto& transformComponent = entityManager.GetRegistry().get<Components::Transform>(camera);
            auto& controlComponent = entityManager.GetRegistry().get<Components::CameraControl>(camera);

            Vector3 move = cameraComponent.forward * controlComponent.speed;
            transformComponent.position -= move;
        });

    input.RegisterKeyCallback(eKeycode_A, eKeyboardKeyState_Pressed,
        [camera]()
        {
            auto& entityManager = Core::EntitiesPool::GetInstance();
            auto& cameraComponent = entityManager.GetRegistry().get<Components::Camera>(camera);
            auto& transformComponent = entityManager.GetRegistry().get<Components::Transform>(camera);
            auto& controlComponent = entityManager.GetRegistry().get<Components::CameraControl>(camera);

            Vector3 move = cameraComponent.right * controlComponent.speed;
            transformComponent.position -= move;
        });

    input.RegisterKeyCallback(eKeycode_D, eKeyboardKeyState_Pressed,
        [camera]()
        {
            auto& entityManager = Core::EntitiesPool::GetInstance();
            auto& cameraComponent = entityManager.GetRegistry().get<Components::Camera>(camera);
            auto& transformComponent = entityManager.GetRegistry().get<Components::Transform>(camera);
            auto& controlComponent = entityManager.GetRegistry().get<Components::CameraControl>(camera);

            Vector3 move = cameraComponent.right * controlComponent.speed;
            transformComponent.position += move;
        });

    // Mouse look
    input.RegisterMouseButtonCallback(eMouseButton_Left, eMouseButtonState_ClickedOnce,
        [camera]()
        {
            auto& entityManager = Core::EntitiesPool::GetInstance();
            auto& cameraComponent = entityManager.GetRegistry().get<Components::Camera>(camera);
            auto& transformComponent = entityManager.GetRegistry().get<Components::Transform>(camera);
            auto& controlComponent = entityManager.GetRegistry().get<Components::CameraControl>(camera);

            auto& mouseManager = MouseManager::GetInstance();

            if (!controlComponent.isRotating)
            {
                controlComponent.isRotating = true;
                return;
            }
        
            Vector2 delta = mouseManager.GetMouseDelta();

            transformComponent.rotationAngles.x -= delta.x * controlComponent.sensetivity;
            transformComponent.rotationAngles.y -= delta.y * controlComponent.sensetivity;

            // Clamp pitch
            transformComponent.rotationAngles.y = std::clamp(transformComponent.rotationAngles.y, -89.0f * 3.14159f / 180.0f, 89.0f * 3.14159f / 180.0f);

            cameraComponent.forward.x = std::cos(transformComponent.rotationAngles.y) * std::cos(transformComponent.rotationAngles.x);
            cameraComponent.forward.y = std::sin(transformComponent.rotationAngles.y);
            cameraComponent.forward.z = std::cos(transformComponent.rotationAngles.y) * std::sin(transformComponent.rotationAngles.x);
            cameraComponent.forward.Normalize();

            Vector3 globalUp{ 0.0f, 1.0f, 0.0f };
            cameraComponent.right = cameraComponent.forward.Cross(globalUp);
            cameraComponent.right.Normalize();
            cameraComponent.up = cameraComponent.right.Cross(cameraComponent.forward);
            cameraComponent.up.Normalize();
        });
    input.RegisterMouseButtonCallback(eMouseButton_Left, eMouseButtonState_Released,
        [camera]()
        {
            auto& entityManager = Core::EntitiesPool::GetInstance();
            auto& controlComponent = entityManager.GetRegistry().get<Components::CameraControl>(camera);

            if (controlComponent.isRotating)
            {
                controlComponent.isRotating = false;
            }
        });
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
    {
        EKeyboardKey key = KeyboardManager::GetInstance().GetPlatformSpecificKeycode(static_cast<uint16_t>(wParam));
        KeyboardManager::GetInstance().SetKeyState(key, eKeyboardKeyState_Pressed);
        break;
    }
    case WM_KEYUP:
    {
        EKeyboardKey key = KeyboardManager::GetInstance().GetPlatformSpecificKeycode(static_cast<uint16_t>(wParam));
        KeyboardManager::GetInstance().SetKeyState(key, eKeyboardKeyState_Released);
        break;
    }
    case WM_LBUTTONDOWN:
    {
        MouseManager::GetInstance().SetButtonState(eMouseButton_Left, eMouseButtonState_ClickedOnce);
        break;
    }
    case WM_LBUTTONUP:
    {
        MouseManager::GetInstance().SetButtonState(eMouseButton_Left, eMouseButtonState_Released);
        break;
    }
    case WM_RBUTTONDOWN:
    {
        MouseManager::GetInstance().SetButtonState(eMouseButton_Right, eMouseButtonState_ClickedOnce);
        break;
    }
    case WM_RBUTTONUP:
    {
        MouseManager::GetInstance().SetButtonState(eMouseButton_Right, eMouseButtonState_Released);
        break;
    }
    case WM_MBUTTONDOWN:
    {
        MouseManager::GetInstance().SetButtonState(eMouseButton_Middle, eMouseButtonState_ClickedOnce);
        break;
    }
    case WM_MBUTTONUP:
    {
        MouseManager::GetInstance().SetButtonState(eMouseButton_Middle, eMouseButtonState_Released);
        break;
    }
    case WM_MOUSEMOVE:
    {
        MouseManager::GetInstance().UpdateMousePosition(Vector2((int)(lParam) & 0xFFFF, ((int)(lParam) >> 16) & 0xFFFF));
        break;
    }

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
