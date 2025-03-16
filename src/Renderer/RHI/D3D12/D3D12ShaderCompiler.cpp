#include "stdafx.h"
#include "D3D12ShaderCompiler.h"
#include <iostream>
#include <fstream>
#include <DebugMacros.h>
#include "D3D12Shader.h"
#include "../RHIContext.h"
#include "D3D12Device.h"

RHI::D3D12ShaderCompiler::D3D12ShaderCompiler()
{
    HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));
    ASSERT(SUCCEEDED(hr), "Failed to create IDxcCompiler instance.");

    hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_library));
    ASSERT(SUCCEEDED(hr), "Failed to create IDxcLibrary instance.");

    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils));
    ASSERT(SUCCEEDED(hr), "Failed to create IDxcUtils instance.");

    hr = m_utils->CreateDefaultIncludeHandler(&m_includeHandler);
    ASSERT(SUCCEEDED(hr), "Failed to create IDxcIncludeHandler instance.");
}

std::shared_ptr<RHI::IShader> RHI::D3D12ShaderCompiler::CompileShader(const ShaderCreateDesription& desc)
{
    RscPtr<IDxcBlobEncoding> sourceBlob;

    HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));
    hr = m_library->CreateBlobFromFile(desc.shaderSourcePath.c_str(), nullptr, &sourceBlob);
    ASSERT(SUCCEEDED(hr), "Failed to create source blob from file: ", desc.shaderSourcePath);

    std::vector<LPCWSTR> compileFlags = {};
    compileFlags.push_back(L"-Zpr"); // sets row-major order

#ifdef DISABLE_SHADER_DEBUGGING
    compileFlags.push_back(L"-Od");
#endif

#ifdef INCLUDE_SHADER_DEBUG_INFO
    compileFlags.push_back(L"-Zi"); // adds debug info

#ifdef STRIP_SHADER_DEBUG_INFO
    compileFlags.push_back(L"-Qstrip_debug");
    compileFlags.push_back(L"-Fd"); // better than /Fo, bcs in case of directory path will create file

    std::filesystem::path pdbFilepath;
    if (desc.shaderPDBPath.empty())
    {
        pdbFilepath = std::filesystem::path(desc.shaderSourcePath).replace_extension("pdb");
    }
    else
    {
        pdbFilepath = std::filesystem::path(desc.shaderPDBPath).replace_filename(desc.shaderSourcePath.filename().replace_extension("pdb"));
    }

    compileFlags.push_back(pdbFilepath.c_str());
#else
    compileFlags.push_back(L"-Qembed_debug");
#endif
    
#endif

#ifdef STRIP_SHADER_REFLECT_INFO
    compileFlags.push_back(L"-Qstrip_reflect");
#endif

    auto& rhiContext = RHIContext::GetInstance();
    ASSERT(rhiContext.GetCurrentAPI() == ERHIRenderingAPI::D3D12, "Shaders target platform doesn't match current API");
    auto d3d12Device = std::static_pointer_cast<D3D12Device>(rhiContext.GetDevice())->m_device;

    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {};
    shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_6;

    ThrowIfFailed(d3d12Device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)));
    ASSERT(shaderModel.HighestShaderModel == D3D_SHADER_MODEL_6_6, "Shader model 6.6 is not supported by this hardware");

    std::wstring shaderProfile = ConvertPipelineStageTypeToD3D12ShaderProfile(desc.pipelineStage);
    shaderProfile += std::to_wstring(shaderModel.HighestShaderModel & 0xF) + L"_";
    shaderProfile += std::to_wstring((shaderModel.HighestShaderModel >> 4) & 0xF);

    RscPtr<IDxcCompilerArgs> compilerArgs;
    m_utils->BuildArguments(
        desc.shaderSourcePath.filename().c_str(),
        desc.entryPoint.c_str(),
        shaderProfile.c_str(),
        compileFlags.data(), static_cast<UINT32>(compileFlags.size()),
        nullptr, 0, // No defines
        &compilerArgs);

    DxcBuffer srcBuffer = {};
    srcBuffer.Ptr = sourceBlob->GetBufferPointer();
    srcBuffer.Size = sourceBlob->GetBufferSize();
    srcBuffer.Encoding = 0;

    RscPtr<IDxcResult> result;
    hr = m_compiler->Compile(&srcBuffer,
        compilerArgs->GetArguments(),
        compilerArgs->GetCount(),
        m_includeHandler.ptr(),
        IID_PPV_ARGS(&result));

    ASSERT(SUCCEEDED(hr), "Shader compilation failed at the compilation stage.");

    HRESULT status;
    result->GetStatus(&status);
    if (FAILED(status))
    {
        RscPtr<IDxcBlobEncoding> errors;
        result->GetErrorBuffer(&errors);
        if (errors)
        {
            std::wcerr << L"Shader compilation failed: "
                << (const char*)errors->GetBufferPointer() << std::endl;
        }
        else
        {
            std::wcerr << L"Shader compilation failed with no additional error info." << std::endl;
        }
        ASSERT(false, "Shader compilation failed");
    }

    RscPtr<IDxcBlob> shaderBlob;
    hr = result->GetResult(&shaderBlob);
    ASSERT(SUCCEEDED(hr), "Failed to retrieve the compiled shader blob.");

    RscPtr<IDxcBlob> pdbBlob;
#ifdef STRIP_SHADER_DEBUG_INFO
    hr = result->GetOutput(
        DXC_OUT_PDB,
        IID_PPV_ARGS(&pdbBlob),
        nullptr);
    ASSERT(SUCCEEDED(hr), "Failed to retrieve shader pdb.");

    std::ofstream pdbFile = std::ofstream(pdbFilepath.c_str(), std::ios::binary);
    pdbFile.write(reinterpret_cast<const char*>(pdbBlob->GetBufferPointer()), pdbBlob->GetBufferSize());
#endif

    RscPtr<IDxcBlob> reflectionBlob;
    hr = result->GetOutput(
        DXC_OUT_REFLECTION,
        IID_PPV_ARGS(&reflectionBlob),
        nullptr);
    ASSERT(SUCCEEDED(hr), "Failed to retrieve shader reflection.");

    RscPtr<ID3D12ShaderReflection> shaderReflection;
    const DxcBuffer reflectionBuffer
    {
        .Ptr = reflectionBlob->GetBufferPointer(),
        .Size = reflectionBlob->GetBufferSize(),
        .Encoding = 0,
    };
    m_utils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection));

    return std::make_shared<D3D12Shader>(shaderBlob, shaderReflection, desc.pipelineStage);
}
