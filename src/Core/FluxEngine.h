#pragma once
#include "../Input/KeyboardKey.h"
#include <SimpleMath.h>
#include "../Utils/RscPtr.h"

class FluxEngine
{
public:
    FluxEngine();
    ~FluxEngine();

    void Init();
    void Update();
    void Render();
    void Destroy();

    void OnKeyDown(EKeyboardKey key) {}
    void OnKeyUp(EKeyboardKey key) {}

    void ParseCommandLineArgs(WCHAR* argv[], int argc);

private:
    
};