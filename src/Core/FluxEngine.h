#pragma once
#include "../Input/KeyboardKey.h"
#include <SimpleMath.h>
#include <RscPtr.h>
#include <CoreModules/Timer.h>

namespace Core
{
    class FluxEngine
    {
    public:
        FluxEngine();
        ~FluxEngine();

        void Init();
        void Update();
        void Render();
        void Destroy();

        void ParseCommandLineArgs(WCHAR* argv[], int argc);

        Timer& GetFrameTimer();

    private:
        Timer m_timer;
    };
}