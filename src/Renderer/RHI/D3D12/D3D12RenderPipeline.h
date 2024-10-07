#pragma once
#include "../RenderPipeline.h"

namespace RHI
{
	struct D3D12RenderPipeline : public IRenderPipeline
	{
		virtual ~D3D12RenderPipeline() {}
	};
}