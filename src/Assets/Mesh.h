#pragma once
#include <vector>
#include "Submesh.h"

namespace Assets
{
	struct StaticMesh
	{
		std::vector<StaticSubmesh> m_submeshes;
	};
}