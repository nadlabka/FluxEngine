#pragma once
#include "../Texture.h"
#include "Managers/DescriptorHeapManager.h"

namespace RHI
{
	struct D3D12Texture : ITexture
	{
		ResourceDescriptorsIndices descriptorsIndices;

		D3D12Texture();
	};
}