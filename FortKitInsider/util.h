#pragma once
#include "framework.h"

namespace Util
{
	static void SetupConsole()
	{
		AllocConsole();

		FILE* pFile;
		freopen_s(&pFile, "CONOUT$", "w", stdout);
		//freopen_s(&pFile, "kkkk.log", "w", stdout);
	}

	static void FixName(std::string& name)
	{
		constexpr static std::string_view chars = " ?+_-:/^()[]<>^.#\'\"%";

		erase_if(name, [](const unsigned char c) { return chars.find(c) != std::string_view::npos; });
	}

	static auto Spacing(const std::string& s, int size = 60)
	{
		const auto spacesC = new char[size++];
		memset(spacesC, ' ', size);
		spacesC[size] = '\0';

		std::string spaces(spacesC);

		if (s.size() > spaces.size())
		{
			return std::string(" ");
		}

		spaces.erase(0, s.size());

		return spaces;
	}

	static bool IsBadReadPtr(void* p)
	{
		MEMORY_BASIC_INFORMATION mbi;
		if (VirtualQuery(p, &mbi, sizeof(mbi)))
		{
			DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
			bool b = !(mbi.Protect & mask);
			if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
				b = true;

			return b;
		}

		return true;
	}
}
