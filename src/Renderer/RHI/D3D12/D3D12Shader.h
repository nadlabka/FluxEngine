#pragma once
#include "../Shader.h"
#include <dxcapi.h>

namespace RHI
{	
	std::wstring ConvertPipelineStageToD3D12ShaderProfile(PipelineStage pipelineStage);

	struct D3D12Shader : public IShader
	{
		D3D12Shader(const ShaderCreateDesription& desc);

		RscPtr<IDxcBlob> m_compiledShader;
		PipelineStage m_pipelineStage;
	};
}