#pragma once
#include <memory>
#include "../../../Renderer/RHI/Texture.h"

namespace Components
{
	struct LightShadowmap
	{
		std::shared_ptr<RHI::ITexture> shadowmap;
	};
}