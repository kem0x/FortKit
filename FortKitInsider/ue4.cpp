#include "ue4.h"

UClass* UClass::StaticClass()
{
	static auto c = FindObject<UClass*>("Class /Script/CoreUObject.Class");
	return c;
}

UClass* UBlueprintGeneratedClass::StaticClass()
{
	static auto c = FindObject<UClass*>("Class /Script/Engine.BlueprintGeneratedClass");
	return c;
}

UClass* UAnimBlueprintGeneratedClass::StaticClass()
{
	static auto c = FindObject<UClass*>("Class /Script/Engine.AnimBlueprintGeneratedClass");
	return c;
}

UClass* UField::StaticClass()
{
	static auto c = FindObject<UClass*>("Class /Script/CoreUObject.Field");
	return c;
}

UClass* UStruct::StaticClass()
{
	static auto c = (UClass*)FindObject<UObject*>("Class /Script/CoreUObject.Struct");
	return c;
}

UClass* UFunction::StaticClass()
{
	static auto c = FindObject<UClass*>("Class /Script/CoreUObject.Function");
	return c;
}

UClass* UEnum::StaticClass()
{
	static auto c = FindObject<UClass*>("Class /Script/CoreUObject.Enum");
	return c;

}

UClass* UScriptStruct::StaticClass()
{
	static auto c = FindObject<UClass*>("Class /Script/CoreUObject.ScriptStruct");
	return c;

}

template <class T>
bool UObject::IsA() const
{
	auto cmp = T::StaticClass();
	if (!cmp->IsValid())
	{
		return false;
	}

	for (auto super = (UStruct*)this->Class; super->IsValid(); super = super->SuperStruct)
	{
		if (super == cmp)
		{
			return true;
		}
	}

	return false;
}

bool UObject::IsA(UClass* cmp) const
{
	if (!cmp->IsValid())
	{
		return false;
	}

	for (auto super = (UStruct*)this->Class; super->IsValid(); super = super->SuperStruct)
	{
		if (super == cmp)
		{
			return true;
		}
	}

	return false;
}

std::string UObject::GetCPPName()
{
	std::string ret;

	if (IsA<UClass>())
	{
		auto c = Cast<UClass*>();
		while (c->IsValid())
		{
			const auto className = c->GetName();
			if (className == "Actor")
			{
				ret += "A";
				break;
			}

			if (className == "Interface")
			{
				ret += "I";
				break;
			}

			if (className == "Object")
			{
				ret += "U";
				break;
			}

			c = c->SuperStruct->Cast<UClass*>();
		}
	}
	else
	{
		ret += "F";
	}

	ret += GetName();

	return ret;
}

bool GlobalObjects::TryFindObject(std::string startOfName, UObject& out)
{
	if (startOfName.empty()) return false;

	for (size_t i = 0; i < ObjectArray.NumElements; i++)
	{
		auto obj = GetByIndex(i);
		if (!obj) continue;

		auto name = obj->GetFullName();

		if (startOfName[0] != name[0]) continue;
		if (name.starts_with(startOfName))
		{
			out = *obj;
			return true;
		}
	}

	return false;
}

bool GlobalObjects::TryFindObjectByName(std::string name, UObject& out)
{
	if (name.empty()) return false;

	for (size_t i = 0; i < ObjectArray.NumElements; i++)
	{
		auto obj = GetByIndex(i);
		if (!obj) continue;

		auto objName = GetNameSafe(obj);

		if (name[0] != objName[0]) continue;
		if (name.compare(objName) == 0)
		{
			out = *obj;
			return true;
		}
	}

	return false;
}

EPropertyType FProperty::GetPropertyType()
{
	switch (this->ClassPrivate->Id)
	{
	case FFieldClassID::Object:
	case FFieldClassID::ObjectPointer:
	case FFieldClassID::Class:
	{
		return EPropertyType::ObjectProperty;
		break;
	}

	case FFieldClassID::Struct:
	{
		return EPropertyType::StructProperty;
		break;
	}

	case FFieldClassID::Int8:
	{
		return EPropertyType::Int8Property;
		break;
	}

	case FFieldClassID::Int16:
	{
		return EPropertyType::Int16Property;
		break;
	}

	case FFieldClassID::Int:
	{
		return EPropertyType::IntProperty;
		break;
	}

	case FFieldClassID::Int64:
	{
		return EPropertyType::Int64Property;
		break;
	}

	case FFieldClassID::UInt16:
	{
		return EPropertyType::UInt16Property;
		break;
	}

	case FFieldClassID::UInt32:
	{
		return EPropertyType::UInt32Property;
		break;
	}

	case FFieldClassID::UInt64:
	{
		return EPropertyType::UInt64Property;
		break;
	}

	case FFieldClassID::Array:
	{
		return EPropertyType::ArrayProperty;
		break;
	}

	case FFieldClassID::Float:
	{
		return EPropertyType::FloatProperty;
		break;
	}

	case FFieldClassID::Double:
	{
		return EPropertyType::DoubleProperty;
		break;
	}

	case FFieldClassID::Bool:
	{
		return EPropertyType::BoolProperty;
		break;
	}

	case FFieldClassID::String:
	{
		return EPropertyType::StrProperty;
		break;
	}

	case FFieldClassID::Name:
	{
		return EPropertyType::NameProperty;
		break;
	}

	case FFieldClassID::Text:
	{
		return EPropertyType::TextProperty;
		break;
	}

	case FFieldClassID::Enum:
	{
		return EPropertyType::EnumProperty;
		break;
	}

	case FFieldClassID::Interface:
	{
		return EPropertyType::InterfaceProperty;
		break;
	}

	case FFieldClassID::Map:
	{
		return EPropertyType::MapProperty;
		break;
	}

	case FFieldClassID::Byte:
	{
		auto bprop = reinterpret_cast<FByteProperty*>(this);

		if (bprop->Enum->IsValid())
		{
			return EPropertyType::EnumAsByteProperty;
		}

		return EPropertyType::ByteProperty;
		break;
	}

	case FFieldClassID::MulticastSparseDelegate:
	{
		return EPropertyType::MulticastDelegateProperty;
		break;
	}
	case FFieldClassID::Delegate:
	{
		return EPropertyType::DelegateProperty;
		break;
	}
	case FFieldClassID::SoftObject:
	case FFieldClassID::SoftClass:
	case FFieldClassID::WeakObject:
	{
		return EPropertyType::SoftObjectProperty;
		break;
	}
	/*case FFieldClassID::WeakObject:
	{
		return EPropertyType::WeakObjectProperty;
		break;
	}*/
	case FFieldClassID::LazyObject:
	{
		return EPropertyType::LazyObjectProperty;
		break;
	}
	case FFieldClassID::Set:
	{
		EPropertyType::SetProperty;
		break;
	}
	default:
	{
		return EPropertyType::Unknown;
	}
	}
}