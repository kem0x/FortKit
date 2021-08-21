#pragma once
#include "framework.h"

constexpr auto OBJECT_PROP_ID = 65536;
constexpr auto STRUCT_PROP_ID = 1048576;
constexpr auto ARRAY_PROP_ID = 2097152;
constexpr auto CLASS_PROP_ID = 1024;
constexpr auto FLOAT_PROP_ID = 256;
constexpr auto BOOL_PROP_ID = 131072;
constexpr auto STR_PROP_ID = 16384;
constexpr auto TEXT_PROP_ID = 1073741824;
constexpr auto NAME_PROP_ID = 8192;
constexpr auto ENUM_PROP_ID = 281474976710656;
constexpr auto BYTE_PROP_ID = 64;
constexpr auto INT_PROP_ID = 128;

namespace Generic
{
	std::string StringifyPropType(FProperty* prop)
	{
		std::string ret;

		switch (prop->ClassPrivate->Id)
		{
		case OBJECT_PROP_ID: case CLASS_PROP_ID:
			{
				ret += "class " + reinterpret_cast<FObjectPropertyBase*>(prop)->PropertyClass->GetCPPName() + "*";
				break;
			}
		case STRUCT_PROP_ID:
			{
				ret += "struct " + reinterpret_cast<FStructProperty*>(prop)->Struct->GetCPPName() + "*";
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
				break;
			}
		case BYTE_PROP_ID:
			{
				break;
			}
		default: ;
		}

		return ret;
	}
}
