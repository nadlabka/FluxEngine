#include <stdafx.h>
#include "FluxEngine.h"
#include <DXSampleHelper.h>
#include "../Application/WinAPI/WinApplication.h"
#include "../Application/WinAPI/WinWindow.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/RHI/RHIContext.h"
#include "./ECS/Entity.h"
#include "../Assets/Mesh.h"
#include <ECS/Components/Transform.h>
#include <ECS/Components/InstancedStaticMesh.h>
#include <ECS/Managers/TransformSystem.h>
#include "../Input/InputManager.h"
#include "../Renderer/Managers/ConstantBufferManager.h"
#include "../Renderer/DataTypes/PerViewConstantBuffer.h"
#include <FillPerViewBuffer.h>
#include "../Renderer/Managers/LightSourcesManager.h"
#include "../Renderer/DataTypes/PerFrameConstantBuffer.h"
#include "../Assets/AssetsLoader.h"
#include <ECS/Components/BehavioralComponents.h>
#include "../Renderer/Managers/MaterialsManager.h"

Core::FluxEngine::FluxEngine()
{
}

Core::FluxEngine::~FluxEngine()
{
}

void Core::FluxEngine::Init()
{
    auto& inputManager = InputManager::GetInstance();
    inputManager.Initialize();

    auto& rhiContext = RHIContext::GetInstance();

    AdapterCreateDesc adapterCreateDesc;
    adapterCreateDesc.useHighPerformanceAdapter = true;
    adapterCreateDesc.useWarpDevice = false;
    DeviceCreateDesc deviceCreateDesc;

    rhiContext.Init(ERHIRenderingAPI::D3D12, adapterCreateDesc, deviceCreateDesc);

    auto& renderer = Renderer::GetInstance();
    renderer.Init();
    renderer.LoadPipeline();
}

void Core::FluxEngine::Update()
{
    InputManager::GetInstance().Update();

    //here we want to propagate transfer to the final children to be reflected in RHI buffer
    auto& transformSystem = TransformSystem::GetInstance();
    auto view = EntitiesPool::GetInstance().GetRegistry().view<Components::Transform, Components::InstancedStaticMesh, Components::CubeComponent>();
    for (auto entity : view)
    {
        auto& transformComponent = view.get<Components::Transform>(entity);
        auto& meshComponent = view.get<Components::InstancedStaticMesh>(entity);
        auto& staticMesh = Assets::AssetsManager<Assets::StaticMesh>::GetInstance().GetAsset(meshComponent.staticMesh);

        transformComponent.rotationAngles.x += 0.001f * m_timer.GetDeltaTime();
        transformComponent.rotationAngles.y += 0.5f * ((uint32_t)(entity) * 10 + 1) * m_timer.GetDeltaTime();
        transformComponent.rotationAngles.z += 0.0001f * m_timer.GetDeltaTime();

        transformSystem.MarkDirty(EntitiesPool::GetInstance().GetRegistry(), entity);
    }

    auto& constantBufferManager = ConstantBufferManager::GetInstance();
    auto cameraView = EntitiesPool::GetInstance().GetRegistry().view<Components::Transform, Components::Camera>();
    for (auto entity : cameraView)
    {
        auto& perViewBuffer = constantBufferManager.GetCpuBuffer<PerViewConstantBuffer>("PerView");
        auto& window = Application::WinApplication::GetWindow();
        FillPerViewBuffer(perViewBuffer, cameraView.get<Components::Transform>(entity), cameraView.get<Components::Camera>(entity), window.GetWidth(), window.GetHeight());
    }
    auto& lightSourcesMgr = LightSourcesManager::GetInstance();
    auto& perFrameBuffer = constantBufferManager.GetCpuBuffer<PerFrameConstantBuffer>("PerFrame");
    perFrameBuffer.pointLightNum = lightSourcesMgr.GetPointLightsNum();
    perFrameBuffer.spotLightNum = lightSourcesMgr.GetSpotLightsNum();
    perFrameBuffer.directionalLightNum = lightSourcesMgr.GetDirectionalLightsNum();
}

void Core::FluxEngine::Render()
{
    auto& transformSystem = TransformSystem::GetInstance();
    transformSystem.UpdateMarkedTransforms(EntitiesPool::GetInstance().GetRegistry());

    auto& renderer = Renderer::GetInstance();
    renderer.Render();
}

void Core::FluxEngine::Destroy()
{
    auto& entityPool = EntitiesPool::GetInstance();
    auto& renderer = Renderer::GetInstance();
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    renderer.WaitForGpu();
    
    ConstantBufferManager::GetInstance().Destroy();

    Assets::AssetsManager<Assets::StaticMesh>::GetInstance().Destroy();
    Assets::AssetsManager<std::shared_ptr<RHI::ITexture>>::GetInstance().Destroy();
    Assets::AssetsManager<std::shared_ptr<RHI::IBuffer>>::GetInstance().Destroy();

    LightSourcesManager::GetInstance().Destroy();

    Assets::AssetsLoader::GetInstance().Destroy();

    MaterialsManager::GetInstance().Destroy();

    entityPool.Destroy();
    renderer.Destroy();

    auto& rhiContext = RHIContext::GetInstance();
    rhiContext.Destroy();
}

void Core::FluxEngine::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
    for (int i = 1; i < argc; ++i)
    {
        if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
            _wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
        {
            Application::WinApplication::SetTitle(Application::WinApplication::GetTitle() + L" (WARP)");
        }
    }
}

Timer& Core::FluxEngine::GetFrameTimer()
{
    return m_timer;
}
