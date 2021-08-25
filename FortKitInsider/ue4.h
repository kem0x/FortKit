#pragma once
#include <locale>
#include "enums.h"
#include "util.h"


template <class T>
struct TArray
{
	friend struct FString;

public:

	T* Data;
	int32_t Count;
	int32_t Max;

	TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	int Num() const
	{
		return Count;
	};

	T& operator[](int i)
	{
		return Data[i];
	};

	const T& operator[](int i) const
	{
		return Data[i];
	};

	bool IsValidIndex(int i) const
	{
		return i < Num();
	}

	void Add(T InputData)
	{
		Data = static_cast<T*>(realloc(Data, sizeof(T) * (Count + 1)));
		Data[Count++] = InputData;
		Max = Count;
	};
};

template <typename KeyType, typename ValueType>
class TPair
{
public:
	KeyType Key;
	ValueType Value;
};

struct FString : private TArray<wchar_t>
{
	FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? (int32_t)std::wcslen(other) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	}

	bool IsValid() const
	{
		return Data != nullptr;
	}

	const wchar_t* ToWString() const
	{
		return Data;
	}

	std::string ToString() const
	{
		auto length = std::wcslen(Data);

		std::string str(length, '\0');

		std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

		return str;
	}
};

inline void (*FNameToString)(void* _this, FString& out);

struct FName
{
	uint32_t ComparisonIndex;
	uint32_t DisplayIndex;

	FName() = default;

	explicit FName(int64_t name)
	{
		DisplayIndex = (name & 0xFFFFFFFF00000000LL) >> 32;
		ComparisonIndex = (name & 0xFFFFFFFFLL);
	};

	auto ToString()
	{
		FString temp;
		FNameToString(this, temp);

		std::string ret(temp.ToString());

		return ret;
	}
};

template <class TEnum>
class TEnumAsByte
{
public:
	TEnumAsByte()
	{
	}

	TEnumAsByte(TEnum _value) : value(static_cast<uint8_t>(_value))
	{
	}

	explicit TEnumAsByte(int32_t _value) : value(static_cast<uint8_t>(_value))
	{
	}

	explicit TEnumAsByte(uint8_t _value) : value(_value)
	{
	}

	operator TEnum() const
	{
		return static_cast<TEnum>(value);
	}

	TEnum GetValue() const
	{
		return static_cast<TEnum>(value);
	}

private:
	uint8_t value;
};

class UObject;

struct FUObjectItem
{
	UObject* Object;
	DWORD Flags;
	DWORD ClusterIndex;
	DWORD SerialNumber;
	DWORD SerialNumber2;
};

struct PreFUObjectItem
{
	FUObjectItem* FUObject[10];
};

struct GlobalObjects
{
	PreFUObjectItem* ObjectArray;
	BYTE unknown1[8];
	int32_t MaxElements;
	int32_t NumElements;

	void NumChunks(int* start, int* end) const
	{
		int cStart = 0, cEnd = 0;

		if (!cEnd)
		{
			while (true)
			{
				if (ObjectArray->FUObject[cStart] == nullptr)
				{
					cStart++;
				}
				else
				{
					break;
				}
			}

			cEnd = cStart;
			while (true)
			{
				if (ObjectArray->FUObject[cEnd] == nullptr)
				{
					break;
				}
				cEnd++;
			}
		}

		*start = cStart;
		*end = cEnd;
	}

	UObject* GetByIndex(int32_t index) const
	{
		int cStart = 0, cEnd = 0;
		int chunkIndex, chunkSize = 0xFFFF, chunkPos;
		FUObjectItem* Object;

		NumChunks(&cStart, &cEnd);

		chunkIndex = index / chunkSize;
		if (chunkSize * chunkIndex != 0 && chunkSize * chunkIndex == index)
		{
			chunkIndex--;
		}

		chunkPos = cStart + chunkIndex;
		if (chunkPos < cEnd)
		{
			Object = ObjectArray->FUObject[chunkPos] + (index - chunkSize * chunkIndex);
			if (!Object) { return nullptr; }

			return Object->Object;
		}

		return nullptr;
	}
};


inline struct GlobalObjects* GObjects;

class UClass;

struct FPointer
{
	uintptr_t Dummy;
};

// UObjectGlobals::StaticLoadObject_Internal
static UObject* (*StaticLoadObject_Internal)(void* Class, void* Outer, const TCHAR* Name, const TCHAR* Filename, uint32_t LoadFlags, void* Sandbox, bool bAllowObjectReconciliation, void* InSerializeContext);

class UObject
{
public:
	FPointer VTableObject;
	DWORD ObjectFlags;
	DWORD InternalIndex;
	UClass* Class;
	FName NamePrivate;
	UObject* Outer;

	auto ProcessEventAddress()
	{
		auto vtable = *reinterpret_cast<void***>(this);
		return vtable[0x44];
	}

	template <typename T>
	static T FindObject(char const* name, bool ends_with = false, int toSkip = 0)
	{
		for (auto i = 0x0; i < GObjects->NumElements; ++i)
		{
			auto object = GObjects->GetByIndex(i);
			if (object == nullptr)
			{
				continue;
			}

			std::string objectFullName = object->GetFullName();

			if (!ends_with)
			{
				if (objectFullName.starts_with(name))
				{
					if (toSkip > 0)
					{
						toSkip--;
					}
					else
					{
						return reinterpret_cast<T>(object);
					}
				}
			}
			else
			{
				if (objectFullName.ends_with(name))
				{
					return reinterpret_cast<T>(object);
				}
			}
		}
		return nullptr;
	}

	auto IsValid() const -> bool
	{
		return (this && !Util::IsBadReadPtr((void*)this));
	}

	template <typename T>
	bool IsA();

	template <typename Base>
	Base Cast() const
	{
		return Base(this);
	}

	std::string GetCPPName();

	auto GetName()
	{
		return NamePrivate.ToString();
	}

	std::string GetFullName()
	{
		std::string temp;

		for (auto outer = Outer; outer; outer = outer->Outer)
		{
			temp = outer->GetName() + "." + temp;
		}

		temp = reinterpret_cast<UObject*>(Class)->GetName() + " " + temp + this->GetName();
		return temp;
	}

	static UClass* StaticClass()
	{
		static auto c = FindObject<UClass*>("Class /Script/CoreUObject.Object");
		return c;
	}
};

class FField;

class FFieldVariant
{
public:
	union FFieldObjectUnion
	{
		FField* Field;
		UObject* Object;
	} Container;

	bool bIsUObject;
};

enum class FFieldClassID : uint64_t
{
	Int8 = 1llu << 1,
	Byte = 1llu << 6,
	Int = 1llu << 7,
	Float = 1llu << 8,
	UInt64 = 1llu << 9,
	Class = 1llu << 10,
	UInt32 = 1llu << 11,
	Interface = 1llu << 12,
	Name = 1llu << 13,
	String = 1llu << 14,
	Object = 1llu << 16,
	Bool = 1llu << 17,
	UInt16 = 1llu << 18,
	Struct = 1llu << 20,
	Array = 1llu << 21,
	Int64 = 1llu << 22,
	Delegate = 1llu << 23,
	SoftObject = 1llu << 27,
	LazyObject = 1llu << 28,
	WeakObject = 1llu << 29,
	Text = 1llu << 30,
	Int16 = 1llu << 31,
	Double = 1llu << 32,
	SoftClass = 1llu << 33,
	Map = 1llu << 46,
	Set = 1llu << 47,
	Enum = 1llu << 48,
	MulticastInlineDelegate = 1llu << 50,
	MulticastSparseDelegate = 1llu << 51
};

class FFieldClass
{
public:

	FName Name;
	FFieldClassID Id;
	uint64_t CastFlags;
	EClassFlags ClassFlags;
	FFieldClass* SuperClass;
	FField* DefaultObject;
};

class FField
{
public:
	FPointer VTableObject;
	FFieldClass* ClassPrivate;
	FFieldVariant Owner;
	FField* Next;
	FName NamePrivate;
	EObjectFlags FlagsPrivate;

	std::string GetName()
	{
		return NamePrivate.ToString();
	}

	std::string GetTypeName() const
	{
		return ClassPrivate->Name.ToString();
	}

	std::string GetFullName()
	{
		std::string temp;

		for (auto outer = Next; outer; outer = outer->Next)
		{
			temp = outer->GetName() + "." + temp;
		}

		temp = GetTypeName() + " " + temp + this->GetName();
		return temp;
	}
};

class FProperty : public FField
{
public:
	int32_t ArrayDim;
	int32_t ElementSize;
	EPropertyFlags PropertyFlags;
	uint16_t RepIndex;
	TEnumAsByte<ELifetimeCondition> BlueprintReplicationCondition;
	int32_t Offset_Internal;
	FName RepNotifyFunc;
	FProperty* PropertyLinkNext;
	FProperty* NextRef;
	FProperty* DestructorLinkNext;
	FProperty* PostConstructLinkNext;
};

class FBoolProperty : public FProperty
{
public:
	uint8_t FieldSize;
	uint8_t ByteOffset;
	uint8_t ByteMask;
	uint8_t FieldMask;

	FORCEINLINE bool IsNativeBool() const
	{
		return FieldMask == 0xff;
	}
};

class FObjectPropertyBase : public FProperty
{
public:

	UClass* PropertyClass;
};

class FArrayProperty : public FProperty
{
public:
	FProperty* Inner;
	EArrayPropertyFlags ArrayFlags;
};

class FStructProperty : public FProperty
{
public:
	class UStruct* Struct;
};

class UField : public UObject
{
public:
	UField* Next;
	void* padding;
	void* padding2;

	static UClass* StaticClass()
	{
		static auto c = FindObject<UClass*>("Class /Script/CoreUObject.Field");
		return c;
	}
};

class UEnum : public UField
{
	enum class ECppForm
	{
		Regular,
		Namespaced,
		EnumClass
	};

public:
	FString CppType;
	TArray<TPair<FName, int64_t>> Names;
	ECppForm CppForm;
};

class FEnumProperty : public FProperty
{
public:
	FProperty* UnderlyingProp;
	UEnum* Enum;
};

class FByteProperty : public FProperty
{
public:
	UEnum* Enum;
};

class FInterfaceProperty : public FProperty
{
public:
	UClass* InterfaceClass;
};

class FMapProperty : public FProperty
{
public:
	FProperty* KeyProp;
	FProperty* ValueProp;
};

class FSetProperty : public FProperty
{
public:
	FProperty* ElementProp;
};

class FSoftClassProperty : public FProperty
{
public:
	UClass* MetaClass;
};

class UStruct : public UField
{
public:
	UStruct* SuperStruct;
	UField* Children;
	FField* ChildProperties;
	int32_t PropertiesSize;
	int32_t MinAlignment;
	TArray<uint8_t> Script;
	FProperty* PropertyLink;
	FProperty* RefLink;
	FProperty* DestructorLink;
	FProperty* PostConstructLink;
	TArray<UObject*> ScriptAndPropertyObjectReferences;
	void /* FUnresolvedScriptPropertiesArray */ * UnresolvedScriptProperties;

	static UClass* StaticClass()
	{
		static auto c = FindObject<UClass*>("Class /Script/CoreUObject.Struct");
		return c;
	}
};

class UClass : public UStruct
{
public:
	static UClass* StaticClass()
	{
		static auto c = FindObject<UClass*>("Class /Script/CoreUObject.Class");
		return c;
	}
};

class UFunction : public UStruct
{
public:
	EFunctionFlags FunctionFlags;
	uint8_t NumParms;
	uint16_t ParmsSize;
	uint16_t ReturnValueOffset; /** Memory offset of return value property */
	uint16_t RPCId; /** Id of this RPC function call (must be FUNC_Net & (FUNC_NetService|FUNC_NetResponse)) */
	uint16_t RPCResponseId; /** Id of the corresponding response call (must be FUNC_Net & FUNC_NetService) */
	FProperty* FirstPropertyToInit;
	UFunction* EventGraphFunction;
	int32_t EventGraphCallOffset;
	void* Func;

	static UClass* StaticClass()
	{
		static auto c = FindObject<UClass*>("Class /Script/CoreUObject.Function");
		return c;
	}
};
