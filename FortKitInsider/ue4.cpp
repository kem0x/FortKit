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