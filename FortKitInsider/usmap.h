#pragma once

#include "writer.h"

class Usmap
{
	FileWriter m_Writer = "Mappings.usmap";
	phmap::flat_hash_map<FName, int /*File Index*/> m_Names;

	struct PropData
	{
		FProperty* Prop;
		uint16_t Index;
		uint8_t ArrayDim;
		FName Name;
		EPropertyType Type;

		PropData(FProperty* p, int idx) :
			Prop(p),
			Index(idx),
			ArrayDim(p->ArrayDim),
			Name(p->NamePrivate),
			Type(p->GetPropertyType())
		{
		}
	};

	void GetStructNames(UStruct*& Struct)
	{
		m_Names.insert_or_assign(Struct->NamePrivate, 0);

		if (Struct->SuperStruct && !m_Names.contains(Struct->SuperStruct->NamePrivate))
			m_Names.insert_or_assign(Struct->SuperStruct->NamePrivate, 0);

		auto prop = (FProperty*)Struct->ChildProperties;

		while (prop)
		{
			m_Names.insert_or_assign(prop->NamePrivate, 0);
			prop = (FProperty*)prop->Next;
		}
	}

	void GetEnumNames(UEnum*& Enum)
	{
		m_Names.insert_or_assign(Enum->NamePrivate, 0);

		for (size_t i = 0; i < Enum->Names.Num(); i++)
		{
			m_Names.insert_or_assign(Enum->Names[i].Key, 0);
		}
	}

	void WritePropertyTypeInfo(FProperty*& Prop, EPropertyType& Type)
	{
		FProperty* Inner = nullptr;
		EPropertyType InnerType = EPropertyType::Unknown;
		FProperty* Value = nullptr;
		EPropertyType ValueType = EPropertyType::Unknown;

		if (Type == EnumAsByteProperty)
		{
			m_Writer.Write(EnumProperty);
		}
		else
		{
			m_Writer.Write(Type);
		}

		switch (Type)
		{
		case EnumProperty:
		{
			Inner = ((FEnumProperty*)Prop)->UnderlyingProp;
			InnerType = Inner->GetPropertyType();
			WritePropertyTypeInfo(Inner, InnerType);
			m_Writer.Write(m_Names[reinterpret_cast<FEnumProperty*>(Prop)->Enum->NamePrivate]);
			break;
		}
		case EnumAsByteProperty:
		{
			m_Writer.Write(ByteProperty);
			m_Writer.Write(m_Names[((FByteProperty*)Prop)->Enum->NamePrivate]);
			break;
		}
		case StructProperty:
		{
			m_Writer.Write(m_Names[((FStructProperty*)Prop)->Struct->NamePrivate]);
			break;
		}
		case SetProperty:
		case ArrayProperty:
		{
			Inner = ((FArrayProperty*)Prop)->Inner;
			InnerType = Inner->GetPropertyType();
			WritePropertyTypeInfo(Inner, InnerType);
			break;
		}
		case MapProperty:
		{
			Inner = ((FMapProperty*)Prop)->KeyProp;
			InnerType = Inner->GetPropertyType();
			WritePropertyTypeInfo(Inner, InnerType);

			Value = ((FMapProperty*)Prop)->ValueProp;
			ValueType = Value->GetPropertyType();
			WritePropertyTypeInfo(Value, ValueType);
			break;
		}
		}
	}

	void HandleProperty(PropData& Prop)
	{
		m_Writer.Write<unsigned short>(Prop.Index);
		m_Writer.Write(Prop.ArrayDim);
		m_Writer.Write(m_Names[Prop.Name]);

		WritePropertyTypeInfo(Prop.Prop, Prop.Type);
	}

public:
	void Generate() // i'm kind of finicky about this code but it's quick and gets the job done
	{
		m_Names.reserve(90000);
		std::vector<UEnum*> Enums;
		Enums.reserve(2500);
		std::vector<UStruct*> Structs;
		Structs.reserve(15000);

		for (size_t i = 0; i < GObjects->ObjectArray.NumElements; i++)
		{
			auto object = GObjects->GetByIndex(i);
			if (object == nullptr)
			{
				continue;
			}

			if (object->Class == UClass::StaticClass() ||
				object->Class == UScriptStruct::StaticClass())
			{
				auto Struct = static_cast<UStruct*>(object);

				if (Struct->Outer && Struct->Outer->Class == UAnimBlueprintGeneratedClass::StaticClass()) //filter weird results
					continue;

				Structs.push_back(Struct);
				GetStructNames(Struct);
			}
			else if (object->Class == UEnum::StaticClass())
			{
				auto Enum = static_cast<UEnum*>(object);
				Enums.push_back(Enum);
				GetEnumNames(Enum);
			}
		}

		m_Writer.Write<uint64_t>(0); //overwrite this later with magic, file size, etc
		m_Writer.Write<uint32_t>(0);

		m_Writer.Write<unsigned int>(m_Names.size());
		int CurrentNameIndex = 0;

		for (auto N : m_Names)
		{
			m_Names[N.first] = CurrentNameIndex; //assign an index to each name in the hash table
			auto Name = N.first.ToString();

			auto Find = Name.find("::");
			if (Find != std::string::npos)
			{
				Name = Name.substr(Find + 2);
			}

			m_Writer.Write<uint8_t>(Name.length());
			m_Writer.WriteString(Name);

			CurrentNameIndex++;
		}

		m_Writer.Write<unsigned int>(Enums.size());

		for (auto Enum : Enums)
		{
			m_Writer.Write(m_Names[Enum->NamePrivate]);
			m_Writer.Write<uint8_t>(Enum->Names.Num());

			for (size_t i = 0; i < Enum->Names.Num(); i++)
			{
				m_Writer.Write<int>(m_Names[Enum->Names[i].Key]);
			}
		}

		m_Writer.Write<unsigned int>(Structs.size());

		for (auto Struct : Structs)
		{
			m_Writer.Write(m_Names[Struct->NamePrivate]);
			m_Writer.Write<int>(Struct->SuperStruct ? m_Names[Struct->SuperStruct->NamePrivate] : 0xffffffff);

			std::vector<PropData> Properties;

			auto Prop = (FProperty*)Struct->ChildProperties;
			unsigned short PropCount = 0;
			unsigned short SerializablePropCount = 0;

			while (Prop)
			{
				PropData Data(Prop, PropCount);

				Properties.push_back(Data);
				Prop = (FProperty*)Prop->Next;

				PropCount += Data.ArrayDim;
				SerializablePropCount++;
			}

			m_Writer.Write(PropCount);
			m_Writer.Write(SerializablePropCount);

			for (auto p : Properties)
			{
				HandleProperty(p);
			}
		}

		m_Writer.Seek(0, SEEK_SET);
		m_Writer.Write<uint16_t>(0x30C4); //magic
		m_Writer.Write<uint8_t>(0); //version
		m_Writer.Write<uint8_t>(0); //compression 
		m_Writer.Write(m_Writer.Size() - 12); //compressed size
		m_Writer.Write(m_Writer.Size() - 12); //decompressed size
	}
};
