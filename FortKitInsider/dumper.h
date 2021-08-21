#pragma once
#include "generic.h"
#include "ue4.h"
#include "util.h"

namespace Dumper
{
	static void DumpClass(UClass* Class)
	{
		if (!Class->ChildProperties && Util::IsBadReadPtr(Class->ChildProperties)) return;

		auto fileType = ".h";
		auto fileName = "DUMP\\" + Class->GetCPPName() + fileType;

		std::ofstream file(fileName);

		file << "struct " << Class->GetCPPName() << (Class->SuperStruct ? " : " + Class->SuperStruct->GetCPPName() : "") << "\n{ \n";

		auto prop = (FProperty*)Class->ChildProperties;

		auto offset = prop->Offset_Internal + prop->ElementSize * prop->ArrayDim;

		//NOT CORRECT
		if (prop->ElementSize > 0x0)
		{
			file << tfm::format("	unsigned char		unreflected_%x[0x%x];\n", prop->ElementSize, prop->ElementSize);
		}

		file << tfm::format("	%s		%s; //0x%x (0x%x)\n", Generic::StringifyPropType(prop), Class->ChildProperties->GetName(), prop->Offset_Internal, prop->ElementSize);

		prop = (FProperty*)Class->ChildProperties->Next;

		if (!prop && Util::IsBadReadPtr(prop)) return;

		while (prop)
		{
			if (offset < prop->Offset_Internal)
			{
				file << tfm::format("	unsigned char		unreflected_%x[0x%x]; //0x%x (0x%x)\n", prop->Offset_Internal, prop->Offset_Internal - offset, prop->Offset_Internal, prop->ElementSize);
			}

			file << tfm::format("	%s		%s; //0x%x (0x%x)\n", Generic::StringifyPropType(prop) + " " + prop->ClassPrivate->Name.ToString() + std::to_string(prop->ClassPrivate->Id), prop->GetName(), prop->Offset_Internal, prop->ElementSize);

			offset = prop->Offset_Internal + prop->ElementSize * prop->ArrayDim;
			prop = (FProperty*)prop->Next;
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
