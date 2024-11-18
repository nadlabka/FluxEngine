#pragma once
#include "../Shader.h"
#include <dxcapi.h>

namespace RHI
{	
	std::wstring ConvertPipelineStageToD3D12ShaderProfile(PipelineStage pipelineStage);

	struct D3D12Shader : public IShader
	{
		D3D12Shader(RscPtr<IDxcBlob> compiledShader, RscPtr<IDxcBlob> pdb, RscPtr<IDxcBlob> reflection, PipelineStage pipelineStage);

		RscPtr<IDxcBlob> m_compiledShader;
		RscPtr<IDxcBlob> m_pdb;
		RscPtr<IDxcBlob> m_reflection;

		PipelineStage m_pipelineStage;
	};
}