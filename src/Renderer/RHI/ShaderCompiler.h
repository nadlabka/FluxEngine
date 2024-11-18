#pragma once
#include <memory>
#include "Shader.h"

namespace RHI
{
	class IShaderCompiler
	{
	public:
		virtual std::shared_ptr<IShader> CompileShader(const ShaderCreateDesription& desc) = 0;
	private:
	};
}