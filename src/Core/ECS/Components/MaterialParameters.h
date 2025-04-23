#pragma once
#include <cstdint>

namespace MaterialParameters
{
	struct UnlitDefault
	{
		uint32_t testParam;
	};

	struct PBRMaterial
	{
		uint32_t albedoIndex = 0xffffffff;
		uint32_t normalIndex = 0xffffffff;
		uint32_t metallicRoughnessIndex = 0xffffffff;
		uint32_t aoIndex = 0xffffffff;
		uint32_t emissiveIndex = 0xffffffff;
	};
}
