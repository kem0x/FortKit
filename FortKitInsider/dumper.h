#pragma once
#include "generic.h"
#include "ue4.h"
#include "util.h"

namespace Dumper
{
	static auto GeneratePadding(const int& size, const int& offset, const std::string& comment, const std::string& name = "unreflected")
	{
		return tfm::format("	unsigned char%s%s_%x[0x%x]; //0x%x (0x%x) %x\n", Util::Spacing("unsigned char"), name, offset, size, offset, size, comment);
	}

	static auto GenerateBitPadding(const int& size, const int& id, const int& offset, const std::string& comment, const std::string& name = "unreflectedBit")
	{
		return tfm::format("	unsigned char%s%s_%x : %d; //0x%x (0x%x) %x\n", Util::Spacing("unsigned char"), name, id, size, offset, size, comment);
	}

	static void GenerateFields(std::ofstream& file, UStruct* Struct)
	{
		auto prop = (FProperty*)Struct->ChildProperties;

		if (Struct->PropertiesSize > 0x0 && !Struct->ChildProperties && Util::IsBadReadPtr(Struct->ChildProperties))
		{
			return (void)(file << tfm::format("	unsigned char%s%s_%x[0x%x];\n", Util::Spacing("unsigned char"), "unreflected", Struct->PropertiesSize, Struct->PropertiesSize));
		}

		auto offset = prop->Offset_Internal + prop->ElementSize * prop->ArrayDim;

		if (Struct->SuperStruct->IsValid() && prop->Offset_Internal >= Struct->SuperStruct->PropertiesSize)
		{
			offset = Struct->SuperStruct->PropertiesSize;
		}

		auto bitFieldsCount = 0;
		FBoolProperty* lastBoolProp = nullptr;

		while (prop)
		{
			if (offset < prop->Offset_Internal)
			{
				lastBoolProp = nullptr;
				file << GeneratePadding(prop->Offset_Internal - offset, offset, "");
			}

			std::string comment;
			auto propName = prop->GetName();
			Util::FixName(propName);

			auto cppType = Generic::StringifyPropType(prop);

			if (!cppType.empty())
			{
				if (prop->ArrayDim > 1)
				{
					propName += tfm::format("[0x%x]", prop->ArrayDim);
					comment += " (ARRAY) ";
				}

				if (prop->ClassPrivate->Id == FFieldClassID::Bool && reinterpret_cast<FBoolProperty*>(prop)->IsBitfield())
				{
					auto bprop = reinterpret_cast<FBoolProperty*>(prop);
					auto missingBits = bprop->GetMissingBitsCount(lastBoolProp);

					if (missingBits.second != -1)
					{
						if (missingBits.first > 0)
						{
							file << GenerateBitPadding(missingBits.first, bitFieldsCount++, lastBoolProp->Offset_Internal, comment);
						}

						if (missingBits.second > 0)
						{
							file << GenerateBitPadding(missingBits.second, bitFieldsCount++, prop->Offset_Internal, comment);
						}
					}
					else if (missingBits.first > 0)
					{
						file << GenerateBitPadding(missingBits.first, bitFieldsCount++, prop->Offset_Internal, comment);
					}

					lastBoolProp = bprop;

					propName += " : 1";
				}
				else
				{
					lastBoolProp = nullptr;
				}

				file << tfm::format("	%s%s%s; //0x%x (0x%x) %s\n", cppType, Util::Spacing(cppType), propName, prop->Offset_Internal, prop->ElementSize, comment);
			}
			else
			{
				//those are ignored on purpose
				if (prop->ClassPrivate->Id != FFieldClassID::MulticastInlineDelegate && prop->ClassPrivate->Id != FFieldClassID::Delegate)
				{
					comment += "(UNHANDLED PROPERTY TYPE: " + prop->ClassPrivate->Name.ToString() + " UID: " + std::to_string((int)prop->ClassPrivate->Id) + ")";
				}

				file << GeneratePadding(prop->ElementSize, prop->Offset_Internal, comment, propName);
			}

			offset = prop->Offset_Internal + prop->ElementSize * prop->ArrayDim;
			prop = (FProperty*)prop->Next;
		}

		if (offset < Struct->PropertiesSize)
		{
			file << tfm::format("	unsigned char%s%s_%x[0x%x]; //0x%x (0x%x)\n", Util::Spacing("unsigned char"), "padding", offset, Struct->PropertiesSize - offset, offset, Struct->PropertiesSize - offset);
		}
	}

	static void GenerateFunctions(std::ofstream& file, UStruct* Struct)
	{
		auto child = Struct->Children;

		if (!child || Util::IsBadReadPtr(child))
		{
			return;
		}

		file << "\n//Functions\n";

		std::vector<std::string> params;

		while (child)
		{
			if (child->IsA(UFunction::StaticClass()))
			{
				auto function = child->Cast<UFunction*>();

				std::string retType;
				std::string functionSig;

				retType = "void";

				auto name = function->GetName();
				Util::FixName(name);

				functionSig += name;

				auto paramChild = function->ChildProperties;

				functionSig += "(";

				while (paramChild)
				{
					auto paramType = Generic::StringifyPropType(reinterpret_cast<FProperty*>(paramChild));

					if (reinterpret_cast<FProperty*>(paramChild)->PropertyFlags & CPF_ReturnParm)
					{
						if (!paramType.empty())
						{
							retType = paramType;
						}

						break;
					}

					if (!paramType.empty())
					{
						if (reinterpret_cast<FProperty*>(child)->ArrayDim > 1)
						{
							paramType += "*";
						}
						else
						{
							paramType += "&";
						}
					}
					else
					{
						break;
					}

					auto paramName = paramChild->GetName();
					Util::FixName(paramName);

					functionSig += paramType + " " + paramName;

					paramChild = paramChild->Next;

					if (paramChild && !(reinterpret_cast<FProperty*>(paramChild)->PropertyFlags & CPF_ReturnParm))
						functionSig += ", ";
				}

				functionSig += ");";

				file << "	" << (function->FunctionFlags & FUNC_Static ? "static " : "") << retType
				<< " " << functionSig
				<< " // " << Generic::StringifyFlags(function->FunctionFlags) << "\n";
			}

			child = child->Next;
		}
	}

	static void DumpStruct(UStruct* Struct)
	{
		if (!Struct->ChildProperties && Util::IsBadReadPtr(Struct->ChildProperties) && Struct->PropertiesSize == 0x0)
			return;

		auto fileType = ".h";
		auto fileName = "DUMP\\" + Struct->GetCPPName() + fileType;

		std::ofstream file(fileName);

		auto isClass = Struct->IsA(UClass::StaticClass());

		file << "// " << Struct->GetFullName();
		file << "\n// " << tfm::format("Size: 0x%x ", Struct->PropertiesSize);
		file << (isClass ? "\nclass " : "\nstruct") << Struct->GetCPPName()
		<< (Struct->SuperStruct ? " : public " + Struct->SuperStruct->GetCPPName() : "")
		<< "\n{ \n" << (isClass ? "public:\n" : "");

		if (Struct->SuperStruct && Struct->PropertiesSize > Struct->SuperStruct->PropertiesSize)
		{
			GenerateFields(file, Struct);
		}

		GenerateFunctions(file, Struct);

		file << "};";

		/*if(Struct->NativeFunctionLookupTable.Num() > 0)
		{
			MessageBoxA(nullptr, Struct->NativeFunctionLookupTable[0].Name.ToString().c_str(), "YAY", MB_OK);
		}*/
	}

	static void Dump()
	{
		printf("[=] Dumping.\n");

		auto Start = std::chrono::steady_clock::now();

		std::ofstream log("Objects.log");

		auto dumped = 0;
		for (auto i = 0x0; i < GObjects->NumElements; ++i)
		{
			auto object = GObjects->GetByIndex(i);
			if (object == nullptr)
			{
				continue;
			}

			std::string objectName = object->GetFullName();
			std::string item = "\n[" + std::to_string(i) + "] Object:[" + objectName + "]\n";
			log << item;

			if (object->IsA(UStruct::StaticClass()))
			{
				DumpStruct((UStruct*)object);
				dumped++;
			}
		}

		auto End = std::chrono::steady_clock::now();
		printf("[+] Dumping done in %.02f ms\n", (End - Start).count() / 1000000.);

		MessageBoxA(nullptr, ("Dumped " + std::to_string(dumped) + " UStruct(s)").c_str(), "Done!", MB_OK);
	}
}
