#pragma once
#include <stdafx.h>
#include "../../Application/WinAPI/WinWindow.h"

namespace RHI
{
	struct ISurface
	{
		virtual ~ISurface() {}

		static std::shared_ptr<ISurface>CreateSurfaceFromWindow(const Application::WinWindow& window);
	};
}