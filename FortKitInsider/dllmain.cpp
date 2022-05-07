#include "consts.h"
#include "core.h"
#include "dumper.h"
#include "framework.h"
#include "memory.h"
#include "util.h"

static void Main(HMODULE hModule)
{
	auto Start = std::chrono::steady_clock::now();

	Util::SetupConsole();

	/*auto StaticLoadObjectInternalAdd = Memory::FindByString(STATICLOADOBJECTINTERNAL_STRINGREF);
	if (!StaticLoadObjectInternalAdd)
	{
		MessageBoxW(nullptr, L"Cannot find StaticLoadObjectInternal.", L"Error", MB_OK);
		return;
	}

	StaticLoadObject_Internal = decltype(StaticLoadObject_Internal)(StaticLoadObjectInternalAdd);*/

	auto GObjectsAdd = Memory::FindPattern("48 8B 05 ? ? ? ? 48 8B 0C C8 48 8D 04 D1 EB 06", true, 3);
	if (!GObjectsAdd)
	{
		MessageBoxW(nullptr, L"Cannot find GObjects.", L"Error", MB_OK);
		return;
	}

	GObjects = decltype(GObjects)(GObjectsAdd);

	auto FNameToStringAdd = Memory::FindByString(FNAMETOSTRING_STRINGREF, { CALL }, true, 1);
	if (!FNameToStringAdd)
	{
		MessageBoxW(nullptr, L"Cannot find FNameToString.", L"Error", MB_OK);
		return;
	}

	FNameToString = decltype(FNameToString)(FNameToStringAdd);

	auto vtable = *reinterpret_cast<void***>(GObjects->GetByIndex(0));
	auto ProcessEventAdd = (uintptr_t)vtable[0x4C];
	ProcessEventR = decltype(ProcessEventR)(ProcessEventAdd);

	auto End = std::chrono::steady_clock::now();

	printf("[=] Init Time: %.02f ms\n", (End - Start).count() / 1000000.);

	Dumper::Dump();
	Dumper::GenerateUsmap();

	FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&Main), hModule, 0, nullptr);

		break;
	}
	default:
		break;
	}
	return TRUE;
}
