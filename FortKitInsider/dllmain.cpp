#include "dumper.h"
#include "framework.h"
#include "memory.h"
#include "util.h"

static void Main(HMODULE hModule)
{
	Util::SetupConsole();

	//auto EngineInitAdd = Memory::FindFunctionByString(L"STAT_FEngineLoop_Init");

	auto GObjectsAdd = Memory::FindPattern("48 8B 05 ? ? ? ? 48 8B 0C C8 48 8D 04 D1 EB 06", true, 3);
	if (!GObjectsAdd)
	{
		MessageBoxW(nullptr, L"Cannot find GObjects pattern.", L"Error", MB_OK);
		return;
	}

	auto FNameToStringAdd = Memory::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 45 33 F6 48 8B F2 44 39 71 04 0F 85 ? ? ? ? 8B 19 0F B7 FB E8 ? ? ? ? 8B CB 48 8D 54 24 ? 48 C1 E9 10 8D 1C 3F 48 03 5C C8 ? 48 8B CB F6 03 01");
	if (!FNameToStringAdd)
	{
		MessageBoxW(nullptr, L"Cannot find FNameToString pattern.", L"Error", MB_OK);
		return;
	}

	GObjects = decltype(GObjects)(GObjectsAdd);
	FNameToString = decltype(FNameToString)(FNameToStringAdd);

	Dumper::DumpClasses();
	//Dumper::DumpGObjects();

	//auto PostRenderAddress = (void*)GObjects->GetByIndex(0)->VTableObject[0x64];
	//PostRender = decltype(PostRender)(PostRenderAddress);

	//FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			Main(hModule);
			break;
		}
	default: break;
	}
	return TRUE;
}
