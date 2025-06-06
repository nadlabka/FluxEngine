#pragma once
#include <filesystem>

namespace RHI
{
	enum class PipelineStageType : uint8_t
	{
		Vertex,
		Fragment,
		Compute
	};

	struct ShaderCreateDesription
	{
		std::filesystem::path shaderSourcePath;
		std::wstring entryPoint;
		PipelineStageType pipelineStage;
		std::filesystem::path shaderPDBPath = {};
		std::vector<std::pair<std::wstring, std::wstring>> defines = {};
	};

	struct IShader
	{
		virtual ~IShader() {}
	};
}