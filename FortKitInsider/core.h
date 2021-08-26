#pragma once
#include "framework.h"

namespace Core
{
	static auto LoadAllClasses()
	{
		static auto AssetRegistryHelpers = UObject::FindObject("AssetRegistryHelpers /Script/AssetRegistry.Default__AssetRegistryHelpers");

		static auto GetAssetRegistry = UObject::FindObject<UFunction*>("Function /Script/AssetRegistry.AssetRegistryHelpers.GetAssetRegistry");

		static UObject* AssetRegistry;

		AssetRegistryHelpers->ProcessEvent(GetAssetRegistry, &AssetRegistry);

		static auto GetAllAssets = UObject::FindObject<UFunction*>("Function /Script/AssetRegistry.AssetRegistry.GetAllAssets");

		struct
		{
			struct TArray<FAssetData> OutAssetData;
			bool bIncludeOnlyOnDiskAssets;
			bool ret;
		} Params;

		Params.bIncludeOnlyOnDiskAssets = false;

		AssetRegistry->ProcessEvent(GetAllAssets, &Params);

		auto loaded = 0;

		for (auto i = 0; i < Params.OutAssetData.Num(); i++)
		{
			auto data = Params.OutAssetData.operator[](i);

			if (data.AssetClass.ToString() == "BlueprintGeneratedClass") /* TODO: move to `data.AssetClass.ToString().ends_with("Class")` */
			{
				if (!UObject::FindObject((data.AssetClass.ToString() + data.ObjectPath.ToString()).c_str())->IsValid())
				{
					if (UObject::StaticLoadObjectEasy(UBlueprintGeneratedClass::StaticClass(), data.ObjectPath.ToWString().c_str())->IsValid())
					{
						loaded++;
						printf("k: %i\n", loaded);
					}
				}
			}
		}

		printf("[+]Loaded %i classes!\n", loaded);
	}

	static auto GetVTableIndex(void* funcAdd)
	{
		auto vtable = GObjects->GetByIndex(0)->GetVTableObject();

		for (auto i = 0x0; i < 0x100; i++)
		{
			auto add = vtable[i];

			if (add && !Util::IsBadReadPtr(add))
			{
				if (add == funcAdd)
				{
					return i;
				}
			}
		}
		
		return -1;
	}
}
