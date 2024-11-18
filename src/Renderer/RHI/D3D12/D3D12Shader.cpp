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

RHI::D3D12Shader::D3D12Shader(RscPtr<IDxcBlob> compiledShader, RscPtr<IDxcBlob> pdb, RscPtr<IDxcBlob> reflection, PipelineStage pipelineStage)
    : m_compiledShader(compiledShader), m_pdb(pdb), m_reflection(reflection), m_pipelineStage(pipelineStage)
{    
}
