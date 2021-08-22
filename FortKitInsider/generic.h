#pragma once
#include "framework.h"

constexpr auto OBJECT_PROP_ID = 65536;
constexpr auto STRUCT_PROP_ID = 1048576;
constexpr auto ARRAY_PROP_ID = 2097152;
constexpr auto CLASS_PROP_ID = 1024;
constexpr auto INTERFACE_PROP_ID = 4096;
constexpr auto FLOAT_PROP_ID = 256;
constexpr auto DOUBLE_PROP_ID = 4294967296;
constexpr auto BOOL_PROP_ID = 131072;
constexpr auto STR_PROP_ID = 16384;
constexpr auto TEXT_PROP_ID = 1073741824;
constexpr auto NAME_PROP_ID = 8192;
constexpr auto ENUM_PROP_ID = 281474976710656;
constexpr auto MAP_PROP_ID = 70368744177664;
constexpr auto SET_PROP_ID = 140737488355328;
constexpr auto MULTICASTS_SPARSE_DELEGATE_PROP_ID = 2251799813685248;
constexpr auto MULTICASTS_INLINE_DELEGATE_PROP_ID = 1125899906842624;
constexpr auto DELEGATE_PROP_ID = 8388608;
constexpr auto SOFTOBJECT_PROP_ID = 536870912;
constexpr auto SOFTCLASS_PROP_ID = 8589934592;
constexpr auto WEAKOBJECT_PROP_ID = 134217728;
constexpr auto LAZYOBJECT_PROP_ID = 268435456;
constexpr auto BYTE_PROP_ID = 64;
constexpr auto INT8_PROP_ID = 2;
constexpr auto INT16_PROP_ID = 2147483648;
constexpr auto INT64_PROP_ID = 4194304;
constexpr auto INT_PROP_ID = 128;
constexpr auto UINT16_PROP_ID = 262144;
constexpr auto UINT32_PROP_ID = 2048;
constexpr auto UINT64_PROP_ID = 512;

namespace Generic
{
	std::string StringifyPropType(FProperty* prop)
	{
		std::string ret;

		switch (prop->ClassPrivate->Id)
		{
		case OBJECT_PROP_ID: case CLASS_PROP_ID:
			{
				ret += "class " + reinterpret_cast<FObjectPropertyBase*>(prop)->PropertyClass->GetCPPName() + (prop->ElementSize == 0x8 ? "*" : "");
				break;
			}
		case STRUCT_PROP_ID:
			{
				ret += "struct " + reinterpret_cast<FStructProperty*>(prop)->Struct->GetCPPName() + (prop->ElementSize == 0x8 ? "*" : "");
				break;
			}
		case INT8_PROP_ID:
			{
				ret += "int8_t";
				break;
			}
		case INT16_PROP_ID:
			{
				ret += "int16_t";
				break;
			}
		case INT_PROP_ID:
			{
				ret += "int";
				break;
			}
		case INT64_PROP_ID:
			{
				ret += "int64_t";
				break;
			}
		case UINT16_PROP_ID:
			{
				ret += "uint16_t";
				break;
			}
		case UINT32_PROP_ID:
			{
				ret += "uint32_t";
				break;
			}
		case UINT64_PROP_ID:
			{
				ret += "uint64_t";
				break;
			}
		case ARRAY_PROP_ID:
			{
				ret += "TArray<" + StringifyPropType(reinterpret_cast<FArrayProperty*>(prop)->Inner) + ">";

				break;
			}
		case FLOAT_PROP_ID:
			{
				ret += "float";
				break;
			}
		case DOUBLE_PROP_ID:
			{
				ret += "double";
				break;
			}
		case BOOL_PROP_ID:
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
		case STR_PROP_ID:
			{
				ret += "struct FString";
				break;
			}
		case NAME_PROP_ID:
			{
				ret += "struct FName";
				break;
			}
		case TEXT_PROP_ID:
			{
				ret += "struct FText";
				break;
			}
		case ENUM_PROP_ID:
			{
				ret += "enum " + reinterpret_cast<FEnumProperty*>(prop)->Enum->GetName();
				break;
			}

		case INTERFACE_PROP_ID:
			{
				ret += "TScriptInterface<class  " + reinterpret_cast<FInterfaceProperty*>(prop)->InterfaceClass->GetCPPName() + ">";
				break;
			}
		case MAP_PROP_ID:
			{
				auto mprop = reinterpret_cast<FMapProperty*>(prop);
				auto keytype = StringifyPropType(mprop->KeyProp);
				auto valuetype = StringifyPropType(mprop->ValueProp);

				if (!keytype.empty() && !valuetype.empty())
				{
					ret += "TMap<" + keytype + ", " + valuetype + ">";
				}
				break;
			}
		case BYTE_PROP_ID:
			{
				auto bprop = reinterpret_cast<FByteProperty*>(prop);

				if (bprop->Enum->IsValid())
				{
					ret += "TEnumAsByte<" + bprop->Enum->GetName() + ">";
				}
				else
				{
					ret += "unsigned char";
				}

				break;
			}
		case MULTICASTS_SPARSE_DELEGATE_PROP_ID:
			{
				ret += "unsigned char";
				break;
			}
			/*
			case MULTICASTS_INLINE_DELEGATE_PROP_ID:
				{
					ret += "FMulticastSparseDelegate";
					break;
				}
			*/
		case SOFTOBJECT_PROP_ID:
			{
				ret += "TWeakObjectPtr<class " + reinterpret_cast<FObjectPropertyBase*>(prop)->PropertyClass->GetCPPName() + ">";
				break;
			}
		case SOFTCLASS_PROP_ID:
			{
				ret += "TWeakObjectPtr<class " + reinterpret_cast<FSoftClassProperty*>(prop)->MetaClass->GetCPPName() + ">";
				break;
			}
		case WEAKOBJECT_PROP_ID:
			{
				ret += "TWeakObjectPtr<class " + reinterpret_cast<FObjectPropertyBase*>(prop)->PropertyClass->GetCPPName() + ">";
				break;
			}
		case LAZYOBJECT_PROP_ID:
			{
				ret += "TLazyObjectPtr<class " + reinterpret_cast<FObjectPropertyBase*>(prop)->PropertyClass->GetCPPName() + ">";
				break;
			}
		case SET_PROP_ID:
			{
				ret += "TSet<" + StringifyPropType(reinterpret_cast<FSetProperty*>(prop)->ElementProp) + ">";
				break;
			}
		default: ;
		}

		return ret;
	}
}
