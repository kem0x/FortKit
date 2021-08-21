#pragma once
#include "ue4.h"
#include "util.h"

namespace Dumper
{
	static void DumpClass(UClass* Class)
	{
		auto fileType = ".h";
		auto fileName = "DUMP\\" + Class->GetCPPName() + fileType;

		std::ofstream file(fileName);

		file << "struct " << Class->GetCPPName() << " \n{\n";

		if (!Class->ChildProperties && Util::IsBadReadPtr(Class->ChildProperties)) return;

		auto next = Class->ChildProperties->Next;

		if (!next && Util::IsBadReadPtr(next)) return;

		file << tfm::format("%s		%s; //0x%d (0x%d)\n", next->GetTypeName(), next->GetName(), reinterpret_cast<FProperty*>(next)->Offset_Internal, reinterpret_cast<FProperty*>(next)->ElementSize);

		while (next)
		{
			file << tfm::format("%s		%s; //0x%d (0x%d)\n", next->GetTypeName(), next->GetName(), reinterpret_cast<FProperty*>(next)->Offset_Internal, reinterpret_cast<FProperty*>(next)->ElementSize);

			next = next->Next;
		}

		file << "};";
	}

	static void DumpClasses()
	{
		printf("[=] Dumping classes.\n");
		std::ofstream log("GObjects.log");

		for (auto i = 0x0; i < GObjects->NumElements; ++i)
		{
			auto object = GObjects->GetByIndex(i);
			if (object == nullptr)
			{
				continue;
			}

			if (object->IsA<UClass>())
			{
				std::string objectName = object->GetFullName();
				std::string item = "\n[" + std::to_string(i) + "] Object:[" + objectName + "]\n";
				log << item;

				DumpClass((UClass*)object);
			}
		}

		printf("[+] Dumping classes is done!\n");
	}


	void DumpGObjects()
	{
		std::ofstream log("GObjects.log");

		for (auto i = 0x0; i < GObjects->NumElements; ++i)
		{
			auto object = GObjects->GetByIndex(i);
			if (object == nullptr)
			{
				continue;
			}
			std::string className = object->Class->GetFullName();
			std::string objectName = object->GetFullName();
			std::string item = "\n[" + std::to_string(i) + "] Object:[" + objectName + "] Class:[" + className + "]\n";
			log << item;
		}
	}
}
