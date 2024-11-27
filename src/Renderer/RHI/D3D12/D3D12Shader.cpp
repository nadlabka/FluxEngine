#include "stdafx.h"
#include "D3D12Shader.h"
#include <dxcapi.h>
#include <iostream>

std::wstring RHI::ConvertPipelineStageToD3D12ShaderProfile(PipelineStage pipelineStage)
{
    switch (pipelineStage)
    {
    case PipelineStage::Vertex:   return L"vs_";
    case PipelineStage::Fragment: return L"ps_";
    case PipelineStage::Compute:  return L"cs_";
    default: throw std::runtime_error("Unsupported pipeline stage");
    }
}

RHI::D3D12Shader::D3D12Shader(RscPtr<IDxcBlob> compiledShader, RscPtr<IDxcBlob> pdb, RscPtr<IDxcBlob> reflection, PipelineStage pipelineStage)
    : m_compiledShader(compiledShader), m_pdb(pdb), m_reflection(reflection), m_pipelineStage(pipelineStage)
{    
}
