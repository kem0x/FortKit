#pragma once
#include "ue4.h"

namespace Dumper
{
	void DumpClass(UClass* Class)
	{
		//auto fileType = L".h";
		//auto fileName = Class->GetName() + fileType;

		//std::wofstream file(fileName);

		MessageBoxW(nullptr, Class->ChildProperties->GetName().c_str(), L"k", MB_OK);

		auto next = Class->ChildProperties->Next;

		auto firstPropertyName = reinterpret_cast<FField*>(Class->ChildProperties)->GetName();

		//file << firstPropertyName << L"\n";

		while (next)
		{
			std::wstring nextName = reinterpret_cast<FField*>(next)->GetName();

			//file << nextName << L"\n";

			if (next->Next)
			{
				next = next->Next;
			}

			//fmt::format("{:d}", "I am not a number");
		}
	}

	void DumpClasses()
	{
		printf("[=] Dumping classes.\n");
		std::wofstream log(L"GObjects.log");

		for (auto i = 0x0; i < GObjects->NumElements; ++i)
		{
			auto object = GObjects->GetByIndex(i);
			if (object == nullptr)
			{
				continue;
			}

			if (object->IsA(UClass::StaticClass()))
			{
				std::wstring objectName = object->GetFullName();
				std::wstring item = L"\n[" + std::to_wstring(i) + L"] Object:[" + objectName + L"]\n";
				log << item;

				DumpClass(reinterpret_cast<UClass*>(object));
			}
		}

		printf("[+] Dumping classes is done!\n");
	}
}
