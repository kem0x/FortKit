#pragma once
#include "generic.h"
#include "ue4.h"
#include "util.h"

namespace Dumper
{
	static auto GeneratePadding(FProperty* prop, const int& size, const int& offset, const std::string& comment, const std::string& name = "unreflected")
	{
		return tfm::format("	unsigned char		%s_%x[0x%x]; //0x%x (0x%x) %x\n", name, offset, size, offset, size, comment);
	}

	static void GenerateFields(std::ofstream& file, UClass* Class)
	{
		auto prop = (FProperty*)Class->ChildProperties;

		if (Class->PropertiesSize > 0x0 && !Class->ChildProperties && Util::IsBadReadPtr(Class->ChildProperties))
		{
			return (void)(file << tfm::format("	unsigned char		%s_%x[0x%x];\n", "unreflected", Class->PropertiesSize, Class->PropertiesSize));
		}

		auto offset = prop->Offset_Internal + prop->ElementSize * prop->ArrayDim;

		file << tfm::format("	%s		%s; //0x%x (0x%x)\n", Generic::StringifyPropType(prop), Class->ChildProperties->GetName(), prop->Offset_Internal, prop->ElementSize);

		prop = (FProperty*)Class->ChildProperties->Next;

		if (!prop && Util::IsBadReadPtr(prop)) return;

		while (prop)
		{
			if (offset < prop->Offset_Internal)
			{
				file << GeneratePadding(prop, prop->Offset_Internal - offset, offset, "");
			}

			std::string comment;
			auto propName = prop->GetName();
			auto cppType = Generic::StringifyPropType(prop);

			if (!cppType.empty())
			{
				if (prop->ArrayDim > 1)
				{
					propName += tfm::format("[0x%x]", prop->ArrayDim);
					comment += " (ARRAY) ";
				}
				file << tfm::format("	%s		%s; //0x%x (0x%x) %s\n", cppType, propName, prop->Offset_Internal, prop->ElementSize, comment);
			}
			else
			{
				//those are ignored on purpose
				if (prop->ClassPrivate->Id != FFieldClassID::MulticastInlineDelegate && prop->ClassPrivate->Id != FFieldClassID::Delegate)
				{
					comment += "(UNHANDLED PROPERTY TYPE: " + prop->ClassPrivate->Name.ToString() + " UID: " + std::to_string((int)prop->ClassPrivate->Id) + ")";
				}

				file << GeneratePadding(prop, prop->ElementSize, prop->Offset_Internal, comment, propName);
			}

			offset = prop->Offset_Internal + prop->ElementSize * prop->ArrayDim;
			prop = (FProperty*)prop->Next;
		}

		if (offset < Class->PropertiesSize)
		{
			file << tfm::format("	unsigned char		%s_%x[0x%x]; //0x%x (0x%x)\n", "padding", offset, Class->PropertiesSize - offset, offset, Class->PropertiesSize - offset);
		}
	}

	static void DumpClass(UClass* Class)
	{
		if (!Class->ChildProperties && Util::IsBadReadPtr(Class->ChildProperties) && Class->PropertiesSize == 0x0) return;

		auto fileType = ".h";
		auto fileName = "DUMP\\" + Class->GetCPPName() + fileType;

		std::ofstream file(fileName);

		file << "struct " << Class->GetCPPName() << (Class->SuperStruct ? " : " + Class->SuperStruct->GetCPPName() : "") << "\n{ \n";

		GenerateFields(file, Class);

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
