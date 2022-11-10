#pragma once
#include "framework.h"

namespace Generic
{
    std::string StringifyPropType(FProperty* prop)
    {
        std::string ret;

        switch (prop->ClassPrivate->Id)
        {
        case FFieldClassID::Object:
        case FFieldClassID::ObjectPointer:
        case FFieldClassID::Class:
        {
            ret += "class " + reinterpret_cast<FObjectPropertyBase*>(prop)->PropertyClass->GetCPPName() + (prop->ElementSize == 0x8 ? "*" : "");
            break;
        }

        case FFieldClassID::Struct:
        {
            ret += "struct " + reinterpret_cast<FStructProperty*>(prop)->Struct->GetCPPName() + (prop->ElementSize == 0x8 ? "*" : "");
            break;
        }

        case FFieldClassID::Int8:
        {
            ret += "int8_t";
            break;
        }

        case FFieldClassID::Int16:
        {
            ret += "int16_t";
            break;
        }

        case FFieldClassID::Int:
        {
            ret += "int";
            break;
        }

        case FFieldClassID::Int64:
        {
            ret += "int64_t";
            break;
        }

        case FFieldClassID::UInt16:
        {
            ret += "uint16_t";
            break;
        }

        case FFieldClassID::UInt32:
        {
            ret += "uint32_t";
            break;
        }

        case FFieldClassID::UInt64:
        {
            ret += "uint64_t";
            break;
        }

        case FFieldClassID::Array:
        {
            ret += "struct TArray<" + StringifyPropType(reinterpret_cast<FArrayProperty*>(prop)->Inner) + ">";

            break;
        }

        case FFieldClassID::Float:
        {
            ret += "float";
            break;
        }

        case FFieldClassID::Double:
        {
            ret += "double";
            break;
        }

        case FFieldClassID::Bool:
        {
            if (reinterpret_cast<FBoolProperty*>(prop)->IsNativeBool())
            {
                ret += "bool";
            }
            else
            {
                ret += "unsigned char";
            }
            break;
        }

        case FFieldClassID::String:
        {
            ret += "struct FString";
            break;
        }

        case FFieldClassID::Name:
        {
            ret += "struct FName";
            break;
        }

        case FFieldClassID::Text:
        {
            ret += "struct FText";
            break;
        }

        case FFieldClassID::Enum:
        {
            ret += "enum " + reinterpret_cast<FEnumProperty*>(prop)->Enum->GetName();
            break;
        }

        case FFieldClassID::Interface:
        {
            ret += "struct TScriptInterface<class " + reinterpret_cast<FInterfaceProperty*>(prop)->InterfaceClass->GetCPPName() + ">";
            break;
        }

        case FFieldClassID::Map:
        {
            auto mprop = reinterpret_cast<FMapProperty*>(prop);
            auto keytype = StringifyPropType(mprop->KeyProp);
            auto valuetype = StringifyPropType(mprop->ValueProp);

            if (!keytype.empty() && !valuetype.empty())
            {
                ret += "struct TMap<" + keytype + ", " + valuetype + ">";
            }
            break;
        }

        case FFieldClassID::Byte:
        {
            auto bprop = reinterpret_cast<FByteProperty*>(prop);

            if (bprop->Enum->IsValid())
            {
                ret += "struct TEnumAsByte<" + bprop->Enum->GetName() + ">";
            }
            else
            {
                ret += "unsigned char";
            }

            break;
        }
        case FFieldClassID::Delegate:
        {
            ret += "struct FDelegate";
            break;
        }
        case FFieldClassID::MulticastInlineDelegate:
        {
            ret += "struct FMulticastInlineDelegate";
            break;
        }
        case FFieldClassID::MulticastSparseDelegate:
        {
            ret += "struct FMulticastSparseDelegate";
            break;
        }
        /*
        case MULTICASTS_INLINE_DELEGATE_PROP_ID:
                {
                        ret += "FMulticastSparseDelegate";
                        break;
                }
        */
        case FFieldClassID::SoftObject:

        {
            ret += "struct TWeakObjectPtr<class " + reinterpret_cast<FObjectPropertyBase*>(prop)->PropertyClass->GetCPPName() + ">";
            break;
        }
        case FFieldClassID::SoftClass:

        {
            ret += "struct TWeakObjectPtr<class " + reinterpret_cast<FSoftClassProperty*>(prop)->MetaClass->GetCPPName() + ">";
            break;
        }
        case FFieldClassID::WeakObject:

        {
            ret += "struct TWeakObjectPtr<class " + reinterpret_cast<FObjectPropertyBase*>(prop)->PropertyClass->GetCPPName() + ">";
            break;
        }
        case FFieldClassID::LazyObject:

        {
            ret += "struct TLazyObjectPtr<class " + reinterpret_cast<FObjectPropertyBase*>(prop)->PropertyClass->GetCPPName() + ">";
            break;
        }
        case FFieldClassID::Set:

        {
            ret += "struct TSet<" + StringifyPropType(reinterpret_cast<FSetProperty*>(prop)->ElementProp) + ">";
            break;
        }
        default:;
        }

        return ret;
    }

    static auto StringifyFlags(uint32_t Flags)
    {
        constexpr static const char* FunctionFlags[32] = { "Final", "0x00000002", "BlueprintAuthorityOnly", "BlueprintCosmetic", "0x00000010", "0x00000020", "Net", "NetReliable", "NetRequest", "Exec", "Native", "Event", "NetResponse", "Static", "NetMulticast", "0x00008000", "MulticastDelegate", "Public", "Private", "Protected", "Delegate", "NetServer", "HasOutParms", "HasDefaults", "NetClient", "DLLImport", "BlueprintCallable", "BlueprintEvent", "BlueprintPure", "0x20000000", "Const", "0x80000000" };

        std::string FlagsA = "(";

        for (int32_t i = 0; i < 32; ++i)
        {
            const uint32_t Mask = 1U << i;
            if ((Flags & Mask) != 0)
            {
                FlagsA += FunctionFlags[i];

                FlagsA += " | ";
            }
        }

        if (FlagsA.size() > 1)
        {
            FlagsA.pop_back();
            FlagsA.pop_back();
            FlagsA.pop_back();
        }

        FlagsA += ")";

        return FlagsA;
    }
}
