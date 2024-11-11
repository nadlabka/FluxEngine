#include "stdafx.h"
#include "D3D12Shader.h"
#include <dxcapi.h>
#include <iostream>

std::wstring RHI::ConvertPipelineStageToD3D12ShaderProfile(PipelineStage pipelineStage)
{
    switch (pipelineStage)
    {
    case PipelineStage::Vertex:   return L"vs_6_0";
    case PipelineStage::Fragment: return L"ps_6_0";
    case PipelineStage::Compute:  return L"cs_6_0";
    }
}

RHI::D3D12Shader::D3D12Shader(const ShaderCreateDesription& desc) 
    : m_pipelineStage(desc.pipelineStage)
{
    RscPtr<IDxcCompiler> compiler;
    RscPtr<IDxcLibrary> library;
    RscPtr<IDxcBlobEncoding> sourceBlob;

    HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
    if (FAILED(hr)) 
    {
        std::cerr << "Failed to create IDxcCompiler instance." << std::endl;
        return;
    }

    hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
    if (FAILED(hr)) 
    {
        std::cerr << "Failed to create IDxcLibrary instance." << std::endl;
        return; 
    }

    hr = library->CreateBlobFromFile(desc.shaderSourcePath.c_str(), nullptr, &sourceBlob);
    if (FAILED(hr) || !sourceBlob) 
    {
        std::cerr << "Failed to create source blob from file: " << desc.shaderSourcePath << std::endl;
        return;
    }

    LPCWSTR* compileFlags = nullptr;
    UINT compileFlagCount = 0;

#ifdef INCLUDE_SHADER_DEBUG_INFO
    LPCWSTR debugFlag = L"-Zi";
    compileFlags = &debugFlag;
    compileFlagCount = 1;
#endif

    RscPtr<IDxcOperationResult> result;
    hr = compiler->Compile(
        sourceBlob.ptr(),
        desc.shaderSourcePath.filename().c_str(),
        desc.entryPoint.c_str(),
        ConvertPipelineStageToD3D12ShaderProfile(desc.pipelineStage).c_str(),
        compileFlags, compileFlagCount,
        nullptr, 0, // No defines
        nullptr, // No include handler
        &result
    );

    if (FAILED(hr) || !result) 
    {
        std::cerr << "Shader compilation failed at the compilation stage." << std::endl;
        return; 
    }

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
        return; 
    }

    RscPtr<IDxcBlob> shaderBlob;
    hr = result->GetResult(&shaderBlob);
    if (FAILED(hr) || !shaderBlob) 
    {
        std::cerr << "Failed to retrieve the compiled shader blob." << std::endl;
        return; 
    }

    m_compiledShader = shaderBlob;
}
