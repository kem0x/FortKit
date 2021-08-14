#pragma once

namespace Memory
{
	//wrote everything here, except the pattern scanner i stole it off guidedhacking
	static void* FindStringRef(const std::wstring& string)
	{
		uintptr_t base_address = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));

		const auto dosHeader = (PIMAGE_DOS_HEADER)base_address;
		const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)base_address + dosHeader->e_lfanew);

		IMAGE_SECTION_HEADER* textSection = nullptr;
		IMAGE_SECTION_HEADER* rdataSection = nullptr;

		auto sectionsSize = ntHeaders->FileHeader.NumberOfSections;
		auto section = IMAGE_FIRST_SECTION(ntHeaders);

		for (WORD i = 0; i < sectionsSize; section++)
		{
			auto secName = std::string((char*)section->Name);

			if (secName == ".text")
			{
				textSection = section;
			}
			else if (secName == ".rdata")
			{
				rdataSection = section;
			}

			if (textSection && rdataSection) break;
		}

		auto textStart = base_address + textSection->VirtualAddress;

		auto rdataStart = base_address + rdataSection->VirtualAddress;
		auto rdataEnd = rdataStart + rdataSection->Misc.VirtualSize;

		const auto scanBytes = reinterpret_cast<std::uint8_t*>(textStart);

		//scan only text section
		for (auto i = 0; i < textSection->Misc.VirtualSize; i++)
		{
			if (scanBytes[i] == 0x4C && scanBytes[i + 1] == 0x8D) //LEA
			{
				auto stringAdd = reinterpret_cast<uintptr_t>(&scanBytes[i]);
				stringAdd = stringAdd + 7 + *reinterpret_cast<int32_t*>(stringAdd + 3);

				//check if the pointer is actually a valid string by checking if it's inside the rdata section
				if (stringAdd > rdataStart && stringAdd < rdataEnd)
				{
					std::wstring lea((const wchar_t*)stringAdd);

					if (lea == string)
					{
						return &scanBytes[i];
					}
				}
			}
		}

		return nullptr;
	}

	static void* FindFunctionByString(const std::wstring& string)
	{
		auto ref = FindStringRef(string);

		if (ref)
		{
			const auto scanBytes = static_cast<std::uint8_t*>(ref);

			//scan backwards till we hit a ret (and assume this is the function start)
			//TODO: find a proper way to scan, i know this is a bad way
			for (auto i = 0; i > -2048; i--)
			{
				if (scanBytes[i] == 0xCC || scanBytes[i] == 0xC3)
				{
					return &scanBytes[i + 1];
				}
			}
		}

		return nullptr;
	}

	static uintptr_t FindPattern(const char* signature, bool bRelative = false, uint32_t offset = 0)
	{
		uintptr_t base_address = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
		static auto patternToByte = [](const char* pattern)
		{
			auto bytes = std::vector<int>{};
			const auto start = const_cast<char*>(pattern);
			const auto end = const_cast<char*>(pattern) + strlen(pattern);

			for (auto current = start; current < end; ++current)
			{
				if (*current == '?')
				{
					++current;
					if (*current == '?') ++current;
					bytes.push_back(-1);
				}
				else { bytes.push_back(strtoul(current, &current, 16)); }
			}
			return bytes;
		};

		const auto dosHeader = (PIMAGE_DOS_HEADER)base_address;
		const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)base_address + dosHeader->e_lfanew);

		const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
		auto patternBytes = patternToByte(signature);
		const auto scanBytes = reinterpret_cast<std::uint8_t*>(base_address);

		const auto s = patternBytes.size();
		const auto d = patternBytes.data();

		for (auto i = 0ul; i < sizeOfImage - s; ++i)
		{
			bool found = true;
			for (auto j = 0ul; j < s; ++j)
			{
				if (scanBytes[i + j] != d[j] && d[j] != -1)
				{
					found = false;
					break;
				}
			}

			if (found)
			{
				uintptr_t address = reinterpret_cast<uintptr_t>(&scanBytes[i]);
				if (bRelative)
				{
					address = ((address + offset + 4) + *(int32_t*)(address + offset));
					return address;
				}
				return address;
			}
		}

		return NULL;
	}
}
