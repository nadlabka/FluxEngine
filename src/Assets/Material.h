#pragma once

namespace Assets
{
	struct DefaultMaterialParameters
	{
		uint32_t m_albedoTexDescriptorIndex;
	};

	struct Material
	{
		std::shared_ptr<RHI::IRenderPipeline> m_PSO;
	};
}