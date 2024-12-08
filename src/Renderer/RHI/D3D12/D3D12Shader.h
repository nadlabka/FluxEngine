#pragma once
#include "../Shader.h"
#include <dxcapi.h>

namespace RHI
{	
	std::wstring ConvertPipelineStageTypeToD3D12ShaderProfile(PipelineStageType pipelineStageType);
	D3D12_SHADER_VISIBILITY ConvertPipelineStageTypeToShaderVisibility(PipelineStageType pipelineStageType);

	struct D3D12Shader : public IShader
	{
		D3D12Shader(RscPtr<IDxcBlob> compiledShader, RscPtr<ID3D12ShaderReflection> reflection, PipelineStageType pipelineStage);

		RscPtr<IDxcBlob> m_compiledShader;
		RscPtr<ID3D12ShaderReflection> m_reflection;

		PipelineStageType m_pipelineStageType;
	};
}