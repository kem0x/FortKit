#pragma once
#include "framework.h"

namespace Injector
{
	static bool InjectDLL(HANDLE Process, const std::string& dllPath)
	{
		auto dllSize = dllPath.length() + 1;

		auto AllocatedMem = VirtualAllocEx(Process, nullptr, dllSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		auto WPM = WriteProcessMemory(Process, AllocatedMem, dllPath.c_str(), dllSize, nullptr);

		auto LoadLibraryAdd = (PTHREAD_START_ROUTINE)GetProcAddress(LoadLibraryA("kernel32"), "LoadLibraryA");

		DWORD ThreadID;
		auto ThreadRet = CreateRemoteThread(Process, nullptr, 0, LoadLibraryAdd, AllocatedMem, 0, &ThreadID);

		WaitForSingleObject(ThreadRet, INFINITE);

		if (AllocatedMem && WPM && ThreadRet)
		{
			return true;
		}

		return false;
	}
}
