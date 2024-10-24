#pragma once
#include "../Shader.h"

namespace RHI
{	
	std::wstring ConvertPipelineStageToD3D12ShaderProfile(PipelineStage pipelineStage);

	struct D3D12Shader : public IShader
	{
		D3D12Shader(const ShaderCreateDesription& desc);

		RscPtr<ID3DBlob> m_compiledShader;
		PipelineStage m_pipelineStage;
	};
}