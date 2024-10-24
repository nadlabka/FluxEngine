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
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
    DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));

    library->CreateBlobFromFile(desc.shaderSourcePath.c_str(), nullptr, &sourceBlob);

    RscPtr<IDxcOperationResult> result;
    compiler->Compile(sourceBlob.ptr(), desc.shaderSourcePath.filename().c_str(),
        desc.entryPoint.c_str(), ConvertPipelineStageToD3D12ShaderProfile(desc.pipelineStage).c_str(),
        nullptr, 0, nullptr, 0, nullptr, &result);

    HRESULT hr;
    result->GetStatus(&hr);
    if (FAILED(hr))
    {
        RscPtr<IDxcBlobEncoding> errors;
        result->GetErrorBuffer(&errors);
        std::wcerr << L"Shader compilation failed: " <<
            (const char*)errors->GetBufferPointer() << std::endl;
    }

    RscPtr<ID3DBlob> shaderBlob;
    result->GetResult(reinterpret_cast<IDxcBlob**>(&shaderBlob));
    m_compiledShader = shaderBlob;
}
