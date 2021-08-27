#pragma once

inline HANDLE gConsole;
inline HANDLE gGameHandle;

#define SET_CONSOLE_COLOR(c) SetConsoleTextAttribute(gConsole, c);

#define printfc(c, ...) SET_CONSOLE_COLOR(c) printf(__VA_ARGS__);

namespace Util
{
	static HANDLE CreateProcessE(const char* lpApplicationName, char* lpArguments)
	{
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		if (CreateProcessA(lpApplicationName, lpArguments, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
		{
			CloseHandle(pi.hThread);
			return pi.hProcess;
		}

		return nullptr;
	}

	static HMODULE GetModule(const HANDLE& handle, const wchar_t* modName)
	{
		HMODULE hMods[1024];
		DWORD cbNeeded;

		if (EnumProcessModules(handle, hMods, sizeof(hMods), &cbNeeded))
		{
			for (auto i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
			{
				TCHAR szModName[MAX_PATH];

				if (GetModuleFileNameEx(handle, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
				{
					if (std::wstring(szModName).ends_with(modName)) return hMods[i];
				}
			}
		}

		return nullptr;
	}

	static std::string GetRuntimePath()
	{
		char result[MAX_PATH];
		std::string path(result, GetModuleFileNameA(nullptr, result, MAX_PATH));
		size_t pos = path.find_last_of("\\/");
		return (std::string::npos == pos) ? "" : path.substr(0, pos);
	}
}
