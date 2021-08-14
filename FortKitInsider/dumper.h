#pragma once
#include "ue4.h"
#include "util.h"

namespace Dumper
{
	void DumpClass(UClass* Class)
	{
		auto fileType = L".h";
		auto fileName = L"DUMP\\" + Class->GetName() + fileType;

		std::wofstream file(fileName);

		file << L"struct " << Class->GetName() << " \n{\n";

		if (!Class->ChildProperties && Util::IsBadReadPtr(Class->ChildProperties)) return;

		auto next = Class->ChildProperties->Next;

		if (!next && Util::IsBadReadPtr(next)) return;

		//MessageBoxW(nullptr, Class->ChildProperties->GetName().c_str(), L"k", MB_OK);

		auto firstPropertyName = reinterpret_cast<FField*>(Class->ChildProperties)->GetName();

		file << firstPropertyName << L"\n";

		while (next)
		{
			file << next->GetTypeName() << L" " << next->GetName() << L";\n";

			next = next->Next;

			//fmt::format("{:d}", "I am not a number");
		}

		file << L"};";
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
