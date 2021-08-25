#include "consts.h"
#include "dumper.h"
#include "framework.h"
#include "memory.h"
#include "util.h"

static void Main(HMODULE hModule)
{
	Util::SetupConsole();

	auto StaticLoadObjectInternalAdd = Memory::FindByString(STATICLOADOBJECTINTERNAL_STRINGREF);
	if (!StaticLoadObjectInternalAdd)
	{
		MessageBoxW(nullptr, L"Cannot find StaticLoadObjectInternal.", L"Error", MB_OK);
		return;
	}

	StaticLoadObject_Internal = decltype(StaticLoadObject_Internal)(StaticLoadObjectInternalAdd);


	//auto GObjectsAdd = Memory::FindPattern("48 8B 05 ? ? ? ? 48 8B 0C C8 48 8D 04 D1 EB 06", true, 3);
	auto GObjectsAdd = Memory::FindByString(GOBJECTS_STRINGREF, {CDQ}, true, 21, true);
	if (!GObjectsAdd)
	{
		MessageBoxW(nullptr, L"Cannot find GObjects.", L"Error", MB_OK);
		return;
	}

	GObjects = decltype(GObjects)(GObjectsAdd);


	auto FNameToStringAdd = (uintptr_t)Memory::FindByString(FNAMETOSTRING_STRINGREF, {CALL}, true, 1);
	if (!FNameToStringAdd)
	{
		MessageBoxW(nullptr, L"Cannot find FNameToString.", L"Error", MB_OK);
		return;
	}

	FNameToString = decltype(FNameToString)(FNameToStringAdd);


	//GObjects->GetByIndex(0)->ProcessEventAddress()
	auto ProcessEventAdd = Memory::FindByString(PROCESSEVENT_STRINGREF, {JMP_REL8}, true, 12, true);
	if (!ProcessEventAdd)
	{
		MessageBoxW(nullptr, L"Cannot find ProcessEventAdd.", L"Error", MB_OK);
		return;
	}

	Dumper::DumpClasses();
	//Dumper::DumpGObjects();

	//FreeLibraryAndExitThread(hModule, 0);
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
