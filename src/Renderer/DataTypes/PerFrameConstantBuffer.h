#pragma once
#include <cstdint>

struct PerFrameConstantBuffer
{
	uint32_t pointLightNum;
	uint32_t spotLightNum;
	uint32_t directionalLightNum;

	uint32_t padding;
};