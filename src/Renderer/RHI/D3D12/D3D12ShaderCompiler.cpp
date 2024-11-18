#include "stdafx.h"
#include "D3D12ShaderCompiler.h"
#include <iostream>
#include <DebugMacros.h>
#include "D3D12Shader.h"

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

#ifdef INCLUDE_SHADER_DEBUG_INFO
    compileFlags.push_back(L"-Zi"); // adds debug info

    compileFlags.push_back(L"-Qstrip_debug");
    compileFlags.push_back(L"-Fd"); // better than /Fo, bcs in case of directory path will create file
    compileFlags.push_back(std::filesystem::path(desc.shaderSourcePath).replace_extension("pdb").c_str());
#endif

#ifdef INCLUDE_SHADER_REFLECTION_INFO
    compileFlags.push_back(L"-Qstrip_reflect");
#endif

    RscPtr<IDxcCompilerArgs> compilerArgs;
    m_utils->BuildArguments(
        desc.shaderSourcePath.filename().c_str(),
        desc.entryPoint.c_str(),
        ConvertPipelineStageToD3D12ShaderProfile(desc.pipelineStage).c_str(),
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
#ifdef INCLUDE_SHADER_DEBUG_INFO
    result->GetOutput(
        DXC_OUT_REFLECTION,
        IID_PPV_ARGS(&pdbBlob),
        nullptr);
#endif

    RscPtr<IDxcBlob> reflectionBlob;
#ifdef INCLUDE_SHADER_REFLECTION_INFO
    result->GetOutput(
        DXC_OUT_REFLECTION,
        IID_PPV_ARGS(&reflectionBlob),
        nullptr);
#endif

    return std::make_shared<D3D12Shader>(shaderBlob, pdbBlob, reflectionBlob, desc.pipelineStage);
}
