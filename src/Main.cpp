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

	Core::FluxEngine engineInstance = Core::FluxEngine();
	Application::WinApplication::Init(&engineInstance, hInstance, nCmdShow, 1920, 1080, L"name");
	Application::WinApplication::Run();

    return 0;
}