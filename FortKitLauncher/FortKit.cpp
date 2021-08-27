#include "framework.h"
#include "injector.h"

auto main() -> int
{
	gConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	//gGameHandle = Launcher::Start();

	auto HWND = FindWindow((L"UnrealWindow"), (L"Fortnite  "));
	if (!IsWindow(HWND))
	{
		printfc(FOREGROUND_RED, "[x] Couldn't find fortnite window, please launch the game with AC off and try again!.");
		return 0;
	}

	DWORD PID;
	GetWindowThreadProcessId(HWND, &PID);

	printfc(FOREGROUND_GREEN, "[+] Found fortnite process, PID: %lu\n", PID);

	gGameHandle = OpenProcess(PROCESS_ALL_ACCESS, false, PID);

	auto dllPath = Util::GetRuntimePath() + R"(\FortKitInsider.dll)";

	if (!Injector::InjectDLL(gGameHandle, dllPath))
	{
		printfc(FOREGROUND_RED, "[x] Couldn't inject the insider into fn process!.\n");
		system("pause");
		return 1;
	}

	printfc(FOREGROUND_BLUE, "[=] Injected successfully, exiting in 5 seconds!.");

	Sleep(5000);
	CloseHandle(gGameHandle);
	return 0;
}
