#include <stdafx.h>
#include "Application/WinAPI/WinWindow.h"
#include "Application/WinAPI/WinApplication.h"
#include "FluxEngine.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	FILE* fp;
	AllocConsole();
	freopen_s(&fp, "CONIN$", "r", stdin);
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);

	FluxEngine engineInstance = FluxEngine();
	WinApplication::Run(&engineInstance, hInstance, nCmdShow, 1280, 720, L"name");

    return 0;
}