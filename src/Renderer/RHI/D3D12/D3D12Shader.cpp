#include "stdafx.h"
#include "D3D12Shader.h"
#include <dxcapi.h>
#include <iostream>

std::wstring RHI::ConvertPipelineStageTypeToD3D12ShaderProfile(PipelineStageType pipelineStageType)
{
    switch (pipelineStageType)
    {
    case PipelineStageType::Vertex:   return L"vs_";
    case PipelineStageType::Fragment: return L"ps_";
    case PipelineStageType::Compute:  return L"cs_";
    default: throw std::runtime_error("Unsupported pipeline stage");
    }
}

D3D12_SHADER_VISIBILITY RHI::ConvertPipelineStageTypeToShaderVisibility(PipelineStageType pipelineStageType)
{
    switch (pipelineStageType)
    {
    case PipelineStageType::Vertex:   return D3D12_SHADER_VISIBILITY_VERTEX;
    case PipelineStageType::Fragment: return D3D12_SHADER_VISIBILITY_PIXEL;
    case PipelineStageType::Compute:  return D3D12_SHADER_VISIBILITY_ALL;
    default: throw std::runtime_error("Unsupported pipeline stage");
    }
}

RHI::D3D12Shader::D3D12Shader(RscPtr<IDxcBlob> compiledShader, RscPtr<ID3D12ShaderReflection> reflection, PipelineStageType pipelineStageType)
    : m_compiledShader(compiledShader), m_reflection(reflection), m_pipelineStageType(pipelineStageType)
{    
}
