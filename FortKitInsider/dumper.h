#pragma once
#include "generic.h"
#include "ue4.h"
#include "util.h"
#include "usmap.h"

namespace Dumper
{
	static void DumpNames()
	{
		std::ofstream NamesDump("NamesDump.txt");

		for (size_t i = 0; i < GObjects->ObjectArray.NumElements; i++)
		{
			auto uObj = GObjects->GetByIndex(i);

			if (Util::IsBadReadPtr(uObj)) continue;

			NamesDump << '[' << uObj->InternalIndex << "] " << uObj->GetFullName() << '\n';
		}

		NamesDump.close();
	}

	static auto GeneratePadding(const int& size, const int& offset, const std::string& comment, const std::string& name = "unreflected")
	{
		std::string cppType = "unsigned char ";
		//Util::Spacing(cppType);

		return tfm::format("	%s%s_%x[0x%x]; //0x%x (0x%x) %x\n", cppType, name, offset, size, offset, size, comment);
	}

	static auto GenerateBitPadding(const int& size, const int& id, const int& offset, const std::string& comment, const std::string& name = "unreflectedBit")
	{
		std::string cppType = "unsigned char ";
		//Util::Spacing(cppType);

		return tfm::format("	%s%s_%x : %d; //0x%x (0x%x) %x\n", cppType, name, id, size, offset, size, comment);
	}

	static void GenerateFields(std::ofstream& file, UStruct* Struct)
	{
		auto prop = (FProperty*)Struct->ChildProperties;

		if (Struct->PropertiesSize > 0x0 && !Struct->ChildProperties && Util::IsBadReadPtr(Struct->ChildProperties))
		{
			std::string cppType = "unsigned char ";
			//Util::Spacing(cppType, 1);

			return (void)(file << tfm::format("	%s%s_%x[0x%x];\n", cppType, "unreflected", Struct->PropertiesSize, Struct->PropertiesSize));
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

			auto cppType = Generic::StringifyPropType(prop, true);

			if (!cppType.empty())
			{
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

				comment += tfm::format("//0x%x (0x%x)", prop->Offset_Internal, prop->ElementSize);

				if (prop->ArrayDim > 1)
				{
					propName += tfm::format("[0x%x]", prop->ArrayDim);
					comment += " (ARRAY) ";
				}

				//Util::Spacing(cppType, 1);
				//Util::Spacing(comment, 50, true);

				//file << tfm::format("	%s%s;", cppType, propName, comment);
				file << tfm::format("	%s%s; %s\n", cppType, propName, comment);
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
			std::string cppType = "unsigned char ";
			//Util::Spacing(cppType);

			auto var = tfm::format("	%s%s_%x[0x%x];", cppType, "padding", offset, Struct->PropertiesSize - offset);
			//Util::Spacing(var, 50, true);

			auto comment = tfm::format(" //0x%x (0x%x)", offset, Struct->PropertiesSize - offset);

			file << var << comment << "\n";
		}
	}

	static void GenerateFunctions(std::ofstream& file, UStruct* Struct)
	{
		auto child = Struct->Children;

		if (!child || Util::IsBadReadPtr(child))
		{
			return;
		}

		file << "\n	/*Functions*/\n";

		std::string params;

		while (child)
		{
			if (child->IsA(UFunction::StaticClass()))
			{
				auto function = child->Cast<UFunction*>();

				std::string retType;
				std::string functionSig;

				retType = "void";

				auto paramChild = function->ChildProperties;

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
						if (reinterpret_cast<FProperty*>(child)->ArrayDim <= 1)
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
					params += paramName;

					paramChild = paramChild->Next;

					if (paramChild && !(reinterpret_cast<FProperty*>(paramChild)->PropertyFlags & CPF_ReturnParm))
						functionSig += ", ";
					params += ", ";
				}

				auto name = function->GetName();
				Util::FixName(name);

				file << "\n	// " << function->GetFullName() << std::endl;
				file << "	" << (function->FunctionFlags & FUNC_Static ? "static " : "") << retType
					<< " " << name << "(" << functionSig << ");"
					<< " // " << Generic::StringifyFlags(function->FunctionFlags) << "\n";

				/*file << "{\n" << "	auto fn = UObject::FindObject(\"" << function->GetFullName() << "\");\n";

				file << "	auto inst = " << "this;\n";

				file << "	return inst->Call<" << retType << ">(fn, " << params << ");";
				file << "\n}\n\n";*/

				params.clear();
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
		file << (isClass ? "\nclass " : "\nstruct ") << Struct->GetCPPName()
			<< (Struct->SuperStruct ? " : public " + Struct->SuperStruct->GetCPPName() : "")
			<< "\n{ \n" << (isClass ? "public:\n" : "");

		if (Struct->SuperStruct)
		{
			if (Struct->PropertiesSize > Struct->SuperStruct->PropertiesSize)
			{
				GenerateFields(file, Struct);
			}
		}
		else GenerateFields(file, Struct);

		GenerateFunctions(file, Struct);

		file << "};";

		/*if(Struct->NativeFunctionLookupTable.Num() > 0)
		{
			MessageBoxA(nullptr, Struct->NativeFunctionLookupTable[0].Name.ToString().c_str(), "YAY", MB_OK);
		}*/
	}

	static void DumpEnum(UEnum* Enum)
	{
		if (!Enum->CppType.IsValid() &&
			Util::IsBadReadPtr((void*)Enum->CppType.ToWString()))
		{
			return;
		}

		auto fileType = ".h";
		auto fileName = "DUMP\\" + Enum->GetCPPString() + fileType; //UObject::GetName can be used too but this is good

		std::ofstream file(fileName);
		file << "// " << Enum->GetFullName() << std::endl;
		file << "enum class " << Enum->GetCPPString() << " : " << Enum->GetEnumType() << std::endl << "{\n";

		for (size_t i = 0; i < Enum->Names.Num(); i++)
		{
			auto name = Enum->Names[i].Key.ToString();
			auto find = name.find(':');

			if (find != std::string::npos)
				name = name.substr(find + 2);

			file << "  " << name << " = " << std::to_string(Enum->Names[i].Value);

			if (i < (Enum->Names.Num() - 1))
			{
				file << ',';
			}

			file << std::endl;
		}

		file << "};";
	}

	static void Dump()
	{
		printf("[=] Dumping.\n");

		auto Start = std::chrono::steady_clock::now();

		std::ofstream log("Objects.log");

		auto dumped = 0;
		auto enumsDumped = 0;
		for (auto i = 0x0; i < GObjects->ObjectArray.NumElements; ++i)
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
				if (object->Class == UFunction::StaticClass())
					continue;

				DumpStruct((UStruct*)object);
				dumped++;
			}
			else if (object->IsA(UEnum::StaticClass()))
			{
				DumpEnum((UEnum*)object);
				enumsDumped++;
			}
		}

		auto End = std::chrono::steady_clock::now();
		printf("[+] Dumping done in %.02f ms\n", (End - Start).count() / 1000000.);

		MessageBoxA(nullptr, ("Dumped " + std::to_string(dumped) + " UStruct(s) and " + std::to_string(enumsDumped) + " UEnum(s)").c_str(), "Done!", MB_OK);
	}

	static void GenerateUsmap()
	{
		printf("[=] Generating Usmap.\n");

		auto Start = std::chrono::steady_clock::now();

		Usmap().Generate();

		auto End = std::chrono::steady_clock::now();

		printf("[+] Usmap file generated in %.02f ms\n", (End - Start).count() / 1000000.);
	}
}
