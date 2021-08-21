#include "ue4.h"

template <typename T> bool UObject::IsA()
{
	auto cmp = T::StaticClass();
	if (!cmp->IsValid())
	{
		return false;
	}

	for (auto super = this->Class; super->IsValid(); super = (UClass*)super->SuperStruct)
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
