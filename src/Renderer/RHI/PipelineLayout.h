#pragma once
#include "PipelineBindings.h"

namespace RHI
{
	struct IPipelineLayout
	{
		IPipelineLayout(PipelineLayoutBindings pipelineLayoutBindings)
			: m_pipelineLayoutBindings(pipelineLayoutBindings) {}
			
		virtual ~IPipelineLayout() {}

		PipelineLayoutBindings m_pipelineLayoutBindings;
	};
}