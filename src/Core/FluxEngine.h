#pragma once
#include "../Input/KeyboardKey.h"
#include <SimpleMath.h>
#include "../Utils/RscPtr.h"

class FluxEngine
{
public:
    FluxEngine();
    ~FluxEngine();

    void OnInit();
    void OnUpdate();
    void OnRender();
    void OnDestroy();

    void OnKeyDown(EKeyboardKey key) {}
    void OnKeyUp(EKeyboardKey key) {}

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

private:
    
};