#pragma once
#include <filesystem>

namespace RHI
{
	enum class PipelineStage : uint8_t
	{
		Vertex,
		Fragment,
		Compute
	};

	struct ShaderCreateDesription
	{
		std::filesystem::path shaderSourcePath;
		std::wstring entryPoint;
		PipelineStage pipelineStage;
	};

	struct IShader
	{
		virtual ~IShader() {}
	};
}