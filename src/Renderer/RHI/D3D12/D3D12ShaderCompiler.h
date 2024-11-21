#pragma once
#include <dxcapi.h>
#include <RscPtr.h>
#include "../ShaderCompiler.h"

namespace RHI
{
	class D3D12ShaderCompiler : public IShaderCompiler
	{
	public:
		D3D12ShaderCompiler();

		std::shared_ptr<IShader> CompileShader(const ShaderCreateDesription& desc);

	private:
		RscPtr<IDxcLibrary> m_library;
		RscPtr<IDxcCompiler3> m_compiler; 
		RscPtr<IDxcUtils> m_utils;
		RscPtr<IDxcIncludeHandler> m_includeHandler;
	};
}