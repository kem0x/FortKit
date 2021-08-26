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

// FName::ToString
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

	auto ToWString()
	{
		FString temp;
		FNameToString(this, temp);

		std::wstring ret(temp.ToWString());

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

struct FPointer
{
	uintptr_t Dummy;
};

// UObjectGlobals::StaticLoadObject_Internal
inline UObject* (*StaticLoadObject_Internal)(void* Class, void* Outer, const TCHAR* Name, const TCHAR* Filename, uint32_t LoadFlags, void* Sandbox, bool bAllowObjectReconciliation, void* InSerializeContext);

// UObject::ProcessEvent
static void* (*ProcessEventR)(void*, void*, void*);

class UObject
{
public:
	FPointer VTableObject;
	DWORD ObjectFlags;
	DWORD InternalIndex;
	class UClass* Class;
	FName NamePrivate;
	UObject* Outer;

	auto GetVTableObject()
	{
		return *reinterpret_cast<void***>(this);
	}

	auto ProcessEvent(void* fn, void* params)
	{
		ProcessEventR(this, fn, params);
	}

	template <typename T = UObject*>
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
		return (!Util::IsBadReadPtr((void*)this));
	}

	static UObject* StaticLoadObjectEasy(UClass* inClass, const wchar_t* inName, UObject* inOuter = nullptr)
	{
		return StaticLoadObject_Internal(inClass, inOuter, inName, nullptr, 0, nullptr, false, nullptr);
	}

	template <class T>
	bool IsA() const;

	template <typename Base>
	Base Cast() const
	{
		return Base(this);
	}

	bool IsA(UClass* cmp) const;

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

	static class UClass* StaticClass()
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

	bool IsBitfield() const { return !IsNativeBool(); }

	static int GetBitPosition(uint8_t value)
	{
		int i4 = !(value & 0xf) << 2;
		value >>= i4;

		int i2 = !(value & 0x3) << 1;
		value >>= i2;

		int i1 = !(value & 0x1);

		int i0 = (value >> i1) & 1 ? 0 : -8;

		return i4 + i2 + i1 + i0;
	}

	std::pair<int, int> GetMissingBitsCount(FBoolProperty* other) const
	{
		if (other == nullptr)
		{
			return {GetBitPosition(ByteMask), -1};
		}

		if (Offset_Internal == other->Offset_Internal)
		{
			return {GetBitPosition(ByteMask) - GetBitPosition(other->ByteMask) - 1, -1};
		}

		return {std::numeric_limits<uint8_t>::digits - GetBitPosition(other->ByteMask) - 1, GetBitPosition(ByteMask)};
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

	static class UClass* StaticClass();
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

	static class UClass* StaticClass();
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

	static class UClass* StaticClass();
};


class UClass : public UStruct
{
public:
	void* ClassConstructor;

	void* ClassVTableHelperCtorCaller;

	void* ClassAddReferencedObjects;

	mutable uint32_t ClassUnique : 31;

	uint32_t bCooked : 1;

	EClassFlags ClassFlags;

	EClassCastFlags ClassCastFlags;

	UClass* ClassWithin;

	UObject* ClassGeneratedBy;

	FName ClassConfigName;

	struct FRepRecord
	{
		FProperty* Property;
		int32_t Index;
	};

	TArray<FRepRecord> ClassReps;

	TArray<UField*> NetFields;

	int32_t FirstOwnedClassRep = 0;

	UObject* ClassDefaultObject;

	void* SparseClassData;

	UStruct* SparseClassDataStruct;

	template <typename Key, typename Value>
	class TMap
	{
		char UnknownData[0x50];
	};

	TMap<FName, UFunction*> FuncMap;

	/** A cache of all functions by name that exist in a parent (superclass or interface) context */
	mutable TMap<FName, UFunction*> SuperFuncMap;

	struct SRWLOCK
	{
		void* Ptr;
	};

	class FWindowsRWLock
	{
	public:
		SRWLOCK Mutex;
	};

	mutable FWindowsRWLock SuperFuncMapLock;

	/**
	 * The list of interfaces which this class implements, along with the pointer property that is located at the offset of the interface's vtable.
	 * If the interface class isn't native, the property will be null.
	 */
	struct FImplementedInterface
	{
		/** the interface class */
		UClass* Class;
		/** the pointer offset of the interface's vtable */
		int32_t PointerOffset;
		/** whether or not this interface has been implemented via K2 */
		bool bImplementedByK2;
	};

	TArray<FImplementedInterface> Interfaces;

	struct FGCReferenceTokenStream
	{
		TArray<uint32_t> Tokens;
		TArray<FName> TokenDebugInfo;
	};

	FGCReferenceTokenStream ReferenceTokenStream;

	struct CRITICAL_SECTION
	{
		void* Opaque1[1];
		long Opaque2[2];
		void* Opaque3[3];
	};

	class FWindowsCriticalSection
	{
		CRITICAL_SECTION CriticalSection;
	};

	FWindowsCriticalSection ReferenceTokenStreamCritical;

	/** This class's native functions. */

	struct FNativeFunctionLookup
	{
		FName Name;
		void* Pointer;
	};

	TArray<FNativeFunctionLookup> NativeFunctionLookupTable;


	static class UClass* StaticClass();
};

class UBlueprintGeneratedClass : UClass
{
public:
	static class UClass* StaticClass();
};


struct FAssetData
{
	struct FName ObjectPath;
	struct FName PackageName;
	struct FName PackagePath;
	struct FName AssetName;
	struct FName AssetClass;
	unsigned char padding_28[0x38];
};
