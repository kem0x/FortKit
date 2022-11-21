#pragma once
#include "generic.h"
#include "ue4.h"
#include "util.h"
#include "usmap.h"

#include "cppgenerator.h"

#include <vector>
#include <algorithm>
#include <iterator>
#include <filesystem>

uintptr_t ModuleBase = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));

namespace Dumper
{
    static void DumpNames()
    {
        std::ofstream NamesDump("NamesDump.txt");

        for (size_t i = 0; i < GObjects->NumElements; i++)
        {
            auto uObj = GObjects->GetByIndex(i);

            if (Util::IsBadReadPtr(uObj))
                continue;

            NamesDump << '[' << uObj->InternalIndex << "] " << uObj->GetFullName() << '\n';
        }

        NamesDump.close();
    }

    static auto GeneratePadding(CPPGenerator::Header& file, const int& size, const int& offset, const std::string& comment, const std::string& name = "unreflected")
    {
        auto var = CPPGenerator::Variable::New<unsigned char>(std::format("{}_{:x}", name, offset));
        var.isArray = true;
        var.arraySize = size;

        var.comment = std::format("0x{:x} (0x{:x}) {}", offset, size, comment);

        return file.variable(var);
    }

    static auto GenerateBitPadding(CPPGenerator::Header& file, const int& size, const int& id, const int& offset, const std::string& name = "unreflectedBit")
    {
        auto var = CPPGenerator::Variable::New<unsigned char>(std::format("{}_{:x}", name, id));

        var.isBitField = true;
        var.bitFieldSize = size;
        var.comment = std::format("0x{:x} (0x{:x})", offset, size);

        return file.variable(var);
    }

    static void GenerateFields(CPPGenerator::Header& file, UStruct* Struct)
    {
        auto prop = (FProperty*)Struct->ChildProperties;

        if (Struct->PropertiesSize > 0x0 && !Struct->ChildProperties && Util::IsBadReadPtr(Struct->ChildProperties))
        {
            auto var = CPPGenerator::Variable::New<unsigned char>(std::format("unreflected_{:x}", Struct->PropertiesSize));
            var.isArray = true;
            var.arraySize = Struct->PropertiesSize;

            return file.variable(var);
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
                GeneratePadding(file, prop->Offset_Internal - offset, offset, "");
            }

            std::string comment;
            auto propName = prop->GetName();
            Util::FixName(propName);

            auto cppType = Generic::StringifyPropType(prop);

            if (!cppType.empty())
            {
                auto var = CPPGenerator::Variable::New(cppType, propName);

                if (lastBoolProp && prop->ClassPrivate->Id == FFieldClassID::Bool && reinterpret_cast<FBoolProperty*>(prop)->IsBitfield())
                {
                    auto bprop = reinterpret_cast<FBoolProperty*>(prop);
                    auto missingBits = bprop->GetMissingBitsCount(lastBoolProp);

                    if (missingBits.second != -1)
                    {
                        if (missingBits.first > 0)
                        {
                            GenerateBitPadding(file, missingBits.first, bitFieldsCount++, lastBoolProp->Offset_Internal);
                        }

                        if (missingBits.second > 0)
                        {
                            GenerateBitPadding(file, missingBits.second, bitFieldsCount++, prop->Offset_Internal);
                        }
                    }
                    else if (missingBits.first > 0)
                    {
                        GenerateBitPadding(file, missingBits.first, bitFieldsCount++, prop->Offset_Internal);
                    }

                    lastBoolProp = bprop;

                    var.isBitField = true;
                    var.bitFieldSize = 1;
                }
                else
                {
                    lastBoolProp = nullptr;
                }

                comment += std::format("0x{:x} (0x{:x})", prop->Offset_Internal, prop->ElementSize);

                if (prop->ArrayDim > 1)
                {
                    var.isArray = true;
                    var.arraySize = prop->ArrayDim;
                    comment += " (ARRAY) ";
                }

                var.comment = comment;
                file.variable(var);
            }
            else
            {
                // those are ignored on purpose
                if (prop->ClassPrivate->Id != FFieldClassID::MulticastInlineDelegate && prop->ClassPrivate->Id != FFieldClassID::Delegate)
                {
                    comment += "(UNHANDLED PROPERTY TYPE: " + prop->ClassPrivate->Name.ToString() + " UID: " + std::to_string((int)prop->ClassPrivate->Id) + ")";
                }

                GeneratePadding(file, prop->ElementSize, prop->Offset_Internal, comment, propName);
            }

            offset = prop->Offset_Internal + prop->ElementSize * prop->ArrayDim;
            prop = (FProperty*)prop->Next;
        }

        if (offset < Struct->PropertiesSize)
        {
            auto var = CPPGenerator::Variable::New<unsigned char>(std::format("padding_{:x}", offset));
            var.isArray = true;
            var.arraySize = Struct->PropertiesSize - offset;
            var.comment = std::format("0x{:x} (0x{:x})", offset, Struct->PropertiesSize - offset);

            file.variable(var);
        }
    }

    static void GenerateFunctions(CPPGenerator::Header& file, UClass* Struct)
    {
        auto child = Struct->Children;

        if (!child || Util::IsBadReadPtr(child))
        {
            return;
        }

        std::vector<FNativeFunctionLookup> nativeFunctions;

        if (Struct->NativeFunctionLookupTable.Num() > 0)
        {
            nativeFunctions = Struct->NativeFunctionLookupTable.ToVector();
        }

        file.AddText("\n\t/* Functions */\n");

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

                    paramChild = (FProperty*)paramChild->Next;

                    if (paramChild && !(paramChild->PropertyFlags & CPF_ReturnParm))
                        functionSig += ", ";
                    params += ", ";
                }

                auto name = function->GetName();
                Util::FixName(name);

                auto nativeFunctionLookup = std::find_if(nativeFunctions.begin(), nativeFunctions.end(),
                    [&](const FNativeFunctionLookup& lookup)
                    {
                        return lookup.Name.ToString().starts_with(function->GetName());
                    });

                std::string nativeFuncComment = " (Has no native function)";

                if (nativeFunctionLookup != std::end(nativeFunctions))
                {
                    nativeFuncComment = std::format(" (Underlying native function: {} 0x{:x})", nativeFunctionLookup->Name.ToString(), (uintptr_t)nativeFunctionLookup->Pointer - ModuleBase);
                    nativeFunctions.erase(nativeFunctionLookup);
                }

                file.AddText(std::format("\n\t// {}{}\n", function->GetFullName(), nativeFuncComment));

                file.AddText(std::format("\t{}{} {}({}); // {}\n", (function->FunctionFlags & FUNC_Static ? "static " : ""), retType, name, functionSig, Generic::StringifyFlags(function->FunctionFlags)));

                params.clear();
            }

            child = child->Next;
        }

        if (nativeFunctions.size() > 0)
        {
            file.AddText("\n\t/* Unreflected Native Functions */\n");

            for (auto&& lookup : nativeFunctions)
            {
                file.AddText(std::format("\t// Function: {} 0x{:x}\n", lookup.Name.ToString(), (uintptr_t)lookup.Pointer - ModuleBase));
            }
        }
    }

    std::unordered_map<std::string, int> names;

    static void DumpStruct(UStruct* Struct)
    {
        if (!Struct->ChildProperties && Util::IsBadReadPtr(Struct->ChildProperties) && Struct->PropertiesSize == 0x0)
            return;

        auto cppname = Struct->GetCPPName();

        CPPGenerator::Header file("DUMP\\" + cppname + (names.contains(cppname) ? "_" + std::to_string(names[cppname]) : ""));
        names[cppname] += 1;

        auto isClass = Struct->IsA(UClass::StaticClass());

        file.AddText(std::format("// {}\n", Struct->GetFullName()));
        file.AddText(std::format("// Size: 0x{:x}\n", Struct->PropertiesSize));

        auto name = Struct->GetCPPName();
        auto parent = Struct->SuperStruct ? Struct->SuperStruct->GetCPPName() : "";

        isClass ? file.classStart(name, parent) : file.structStart(name, parent);

        if (Struct->SuperStruct)
        {
            if (Struct->PropertiesSize > Struct->SuperStruct->PropertiesSize)
            {
                GenerateFields(file, Struct);
            }
        }
        else
            GenerateFields(file, Struct);

        if (isClass)
        {
            GenerateFunctions(file, (UClass*)Struct);
        }

        isClass ? file.classEnd() : file.structEnd();
    }

    static void DumpEnum(UEnum* Enum)
    {
        if (!Enum->CppType.IsValid() && Util::IsBadReadPtr((void*)Enum->CppType.ToWString()))
        {
            return;
        }
        
        auto cppname = Enum->GetCPPString();

        auto fileType = ".h";
        auto fileName = "DUMP\\" + cppname + (names.contains(cppname) ? "_" + std::to_string(names[cppname]) : "") + fileType; // UObject::GetName can be used too but this is good
        names[cppname] += 1;

        std::ofstream file(fileName);
        file << "// " << Enum->GetFullName() << std::endl;
        file << "enum class " << Enum->GetCPPString() << " : " << Enum->GetEnumType() << std::endl
             << "{\n";

        for (int i = 0; i < Enum->Names.Num(); i++)
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

        std::filesystem::create_directories("DUMP");

        auto Start = std::chrono::steady_clock::now();

        auto dumped = 0;
        auto enumsDumped = 0;
        for (auto i = 0x0; i < GObjects->NumElements; ++i)
        {
            auto object = GObjects->GetByIndex(i);
            if (object == nullptr)
            {
                continue;
            }

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
