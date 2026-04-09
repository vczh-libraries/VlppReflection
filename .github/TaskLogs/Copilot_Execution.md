# !!!EXECUTION!!!

# UPDATES

# AFFECTED PROJECTS
- Always Build the solution in folder REPO-ROOT\Test\UnitTest with Debug|Win32.
- Always Run CLI|UnitTest project Metadata_Generate with Debug|Win32.
- Always Build the solution in folder REPO-ROOT\Test\UnitTest with Debug|x64.
- Always Run CLI|UnitTest project Metadata_Generate with Debug|x64.
- Always Run CLI|UnitTest project Metadata_Test with Debug|x64.
- Always Run CLI|UnitTest project Metadata_Test with Debug|Win32 (extra coverage required by the task; Project.md only requires x64).
- Always Run CLI|UnitTest project VlppReflection with Debug|x64.

# EXECUTION PLAN

## STEP 1: Implement runtime attribute storage (non-metaonly)

### Files to change
- Source\Reflection\Metadata\Metadata.h
- Source\Reflection\Metadata\Metadata.cpp

### 1.1 Add a concrete IAttributeInfo implementation
Add a class (declare in `Metadata.h` near other impl types; implement in `Metadata.cpp`):

```
class AttributeInfoImpl : public Object, public IAttributeInfo
{
protected:
	ITypeDescriptor*				attributeType = nullptr;
	collections::List<Value>	values;
public:
	AttributeInfoImpl(ITypeDescriptor* _attributeType);
	ITypeDescriptor* GetAttributeType()override;
	vint GetAttributeValueCount()override;
	Value GetAttributeValue(vint index)override;
	void AddValue(const Value& value);
};
```

### 1.2 Make AttributeBagSource usable for both runtime + metaonly descriptors
`MetaonlyTypeDescriptor` inherits from `AttributeBagSource` directly (see `Source\Reflection\DescriptableInterfaces_Metaonly.cpp`). Therefore **attribute storage + retrieval + registration must live on `AttributeBagSource`**, not only on `TypeDescriptorImplBase`.

In `Metadata.h`, update `AttributeBagSource` to:
- own attribute storage + “last registered” tracking
- provide virtual dispatch points with safe defaults (0 / nullptr)
- provide registration helpers used by reflection macros and by metaonly-loading code

Required shape (names can vary, behavior must match):

```
class AttributeBagSource : public Object
{
	template<typename TMemberInfo>
	friend class MemberInfoBase;

protected:
	collections::List<Ptr<IAttributeInfo>>						typeAttributes;
	collections::Group<IMemberInfo*, Ptr<IAttributeInfo>>		memberAttributes;
	IMemberInfo*	lastRegisteredMember = nullptr;
	IMethodInfo*	lastRegisteredMethod = nullptr;

	virtual vint GetAttributeCountInternal(IMemberInfo* memberInfo)
	{
		if (!memberInfo)
		{
			return typeAttributes.Count();
		}
		return memberAttributes.GetByKey(memberInfo).Count();
	}

	virtual IAttributeInfo* GetAttributeInternal(IMemberInfo* memberInfo, vint index)
	{
		if (!memberInfo)
		{
			return typeAttributes[index].Obj();
		}
		return memberAttributes.GetByKey(memberInfo)[index].Obj();
	}

public:
	// Must be callable from:
	// - reflection macro authoring (inside LoadInternal)
	// - metaonly metadata loading code (LoadMetaonlyTypes)
	void RegisterTypeAttribute(Ptr<IAttributeInfo> info)
	{
		typeAttributes.Add(info);
	}

	void RegisterMemberAttribute(IMemberInfo* memberInfo, Ptr<IAttributeInfo> info)
	{
		CHECK_ERROR(memberInfo, L"... memberInfo should not be nullptr.");
		memberAttributes.Add(memberInfo, info);
	}

	IMemberInfo* GetLastRegisteredMember()const
	{
		return lastRegisteredMember;
	}

	IMethodInfo* GetLastRegisteredMethod()const
	{
		return lastRegisteredMethod;
	}

	void SetLastRegisteredMember(IMemberInfo* member)
	{
		lastRegisteredMember = member;
		lastRegisteredMethod = nullptr;
	}

	void SetLastRegisteredMethod(IMethodInfo* method)
	{
		lastRegisteredMethod = method;
		lastRegisteredMember = method;
	}

	void ClearLastRegisteredMethod()
	{
		lastRegisteredMethod = nullptr;
	}
};
```

Notes:
- Keep behavior deterministic: insertion order in `typeAttributes` and per-`memberInfo` list must be preserved.
- Virtual dispatch points remain overridable, but the default implementation should work for both runtime descriptors (`TypeDescriptorImplBase`) and metaonly descriptors (`MetaonlyTypeDescriptor`).

### 1.3 Ensure “last registered” tracking is maintained by existing registration helpers
In `Metadata.cpp` (where `AddProperty/AddEvent/AddMethod/AddConstructor` are implemented), keep tracking consistent:
- After `AddMethod` / `AddConstructor`: call `SetLastRegisteredMethod(returnedMethod)`.
- After `AddProperty` / `AddEvent`: call `SetLastRegisteredMember(returnedMember)`.

This also ensures `CLASS_MEMBER_FIELD` is covered (it goes through `AddProperty(...)`).

## STEP 2: Implement macro authoring API (ATTRIBUTE_TYPE / ATTRIBUTE_MEMBER / ATTRIBUTE_PARAMETER)

### Files to change
- Source\Reflection\Reflection\Macros.h
- Source\Reflection\Metadata\Metadata.h (for struct-field tracking support)

### 2.1 Add helper templates for attribute construction + validation
Add helper(s) in `Macros.h` (or a nearby header included by it) to build `Ptr<IAttributeInfo>` and validate requirements.

```
namespace vl::reflection::description::detail
{
	namespace attribute_macro
	{
		template<typename T>
		Value BoxAttributeArgument(T&& arg)
		{
			using A = std::remove_reference_t<T>;
			if constexpr (std::is_array_v<A> && std::is_same_v<std::remove_cv_t<std::remove_extent_t<A>>, wchar_t>)
			{
				// Allow L"..." in ATTRIBUTE_* and box it as WString (matches task example).
				// NOTE: WString::Unmanaged is safe here because boxing copies the value immediately.
				return BoxValue<WString>(WString::Unmanaged(arg));
			}
			else if constexpr (std::is_same_v<std::remove_cvref_t<T>, const wchar_t*>)
			{
				return BoxValue<WString>(WString::Unmanaged(arg));
			}
			else
			{
				return BoxValue<std::remove_cvref_t<T>>(std::forward<T>(arg));
			}
		}

		template<typename T>
		void AddAttributeArgument(AttributeInfoImpl* info, T&& arg)
		{
			Value v = BoxAttributeArgument(std::forward<T>(arg));
			auto vtd = v.GetTypeDescriptor();
			CHECK_ERROR(vtd && vtd->GetSerializableType(), L"... attribute argument must be serializable.");
			info->AddValue(v);
		}

		template<typename TAttribute, typename... TArgs>
		Ptr<IAttributeInfo> MakeAttributeInfo(TArgs&&... args)
		{
			static_assert(requires { TAttribute{ std::forward<TArgs>(args)... }; });

			auto attrTd = description::GetTypeDescriptor<TAttribute>();
			CHECK_ERROR(attrTd, L"... GetTypeDescriptor<TAttribute>() failed.");
			CHECK_ERROR(attrTd->GetTypeDescriptorFlags() == TypeDescriptorFlags::Struct, L"... attribute type must be a struct.");

			auto info = Ptr(new AttributeInfoImpl(attrTd));
			(AddAttributeArgument(info.Obj(), std::forward<TArgs>(args)), ...);
			return info;
		}
	}
}
```

### 2.2 Define the 3 macros
Define in `Macros.h` so they can appear inside `LoadInternal()` between `BEGIN_*_MEMBER` and `END_*_MEMBER`.

- `ATTRIBUTE_TYPE(TYPE, ...)`: register on the current type descriptor (`this`)
- `ATTRIBUTE_MEMBER(TYPE, ...)`: register on `this->GetLastRegisteredMember()` (must exist)
- `ATTRIBUTE_PARAMETER(PARAMETER_NAME, TYPE, ...)`: register on the named parameter of `this->GetLastRegisteredMethod()` (must exist)

Implementation requirements:
- Must be available in both full-reflection and metaonly-reflection builds: gate only by `#ifndef VCZH_DEBUG_NO_REFLECTION` (do **not** exclude `VCZH_DEBUG_METAONLY_REFLECTION`).
- 0 args must work (`TYPE{}`).
- Wide string literals like `L"name"` must be supported (box to `vl::WString`).
- Misuse must fail with clear `CHECK_ERROR` messages.
- `ATTRIBUTE_PARAMETER` must `CHECK_ERROR` when:
  - there is no `lastRegisteredMethod`, or
  - no parameter matches `PARAMETER_NAME`, or
  - **more than one** parameter matches `PARAMETER_NAME` (ambiguous duplicate names).

### 2.3 Support ATTRIBUTE_MEMBER after STRUCT_MEMBER
Update `StructTypeDescriptor` so struct fields also update last-member tracking.

In `Source\Reflection\Metadata\Metadata.h` inside `StructTypeDescriptor`:
- add `IPropertyInfo* AddField(Ptr<IPropertyInfo> value)` that appends to `fields`, sets lastRegisteredMember, clears lastRegisteredMethod.

In `Source\Reflection\Reflection\Macros.h`, change `STRUCT_MEMBER` to call `AddField`:

```
#define STRUCT_MEMBER(FIELDNAME) \
	AddField(Ptr(new StructFieldInfo<decltype(((StructType*)0)->FIELDNAME)>(this, &StructType::FIELDNAME, L ## #FIELDNAME)));
```


## STEP 3: Extend metaonly metadata schema to serialize/deserialize attributes

### File to change
- Source\Reflection\DescriptableInterfaces_Metaonly.cpp

### 3.1 Add metadata structs
Add near existing metadata structs:

```
struct AttributeValueMetadata
{
	WString typeName;
	WString data;
};

struct AttributeInfoMetadata
{
	WString attributeTypeName;
	List<AttributeValueMetadata> values;
};
```

Learning / clarity requirement:
- When constructing these aggregate-like metadata payload structs in code, prefer C++20 designated initializers to avoid positional-init mistakes.


### 3.2 Extend existing metadata structs
Add `List<AttributeInfoMetadata> attributes;` to:
- TypeDescriptorMetadata
- MethodInfoMetadata
- PropertyInfoMetadata
- EventInfoMetadata
- ParameterInfoMetadata

### 3.3 Update serialization declarations
Update the `BEGIN_SERIALIZATION(...)` sections to include the new fields/structs.

### 3.4 Generate attributes into metadata
In `GenerateMetaonly*` routines:
- For each `IAttributeBag` (type, members, parameters), iterate `GetAttributeCount()`.
- For each `IAttributeInfo`:
  - `attributeTypeName = info->GetAttributeType()->GetTypeName()`
  - For each argument value: serialize using `ISerializableType::Serialize` and record the value type name and serialized data.

### 3.5 Load attributes from metadata
In `LoadMetaonlyTypes()`:
- After all metaonly objects are created (type descriptors, methods, properties, events, parameters), load and register attributes.
- For each `AttributeInfoMetadata`:
  - Resolve `attributeTypeName` to an `ITypeDescriptor*` **from the loaded metaonly type descriptors** (e.g., build a lookup table from `context->tds`).
    - `CHECK_ERROR` if the attribute type cannot be resolved.
  - Construct `AttributeInfoImpl(attrTd)`.
  - For each `AttributeValueMetadata`:
    - Find `ISerializableType*` by `value.typeName` from `context->serializableTypes`.
      - `CHECK_ERROR` if missing.
    - Call `Deserialize(value.data, outValue)` and `CHECK_ERROR` if deserialization fails.
    - Add the `Value` to `AttributeInfoImpl` in order.
  - Register onto the owning bag (`AttributeBagSource`) using `RegisterTypeAttribute` / `RegisterMemberAttribute`.

Error semantics (must be explicit in code):
- `CHECK_ERROR` when an attribute type cannot be resolved.
- `CHECK_ERROR` when a serializable type cannot be resolved.
- `CHECK_ERROR` when deserialization fails.

Note: This is an intentional binary schema change; `.bin/.txt` must be regenerated by `Metadata_Generate` and then verified by `Metadata_Test`.


## STEP 4: Update LogTypeManager text output to include attributes

### File to change
- Source\Reflection\DescriptableInterfaces_Log.cpp

Add deterministic attribute printing for:
- Type descriptors
- Members (property/event/method/constructor/field)
- Method parameters

Explicit non-goals:
- Method groups remain non-attributable; keep `MethodGroupInfoImpl::GetAttributeCount/GetAttribute` as `0` / `nullptr`, and do not print any group-level attribute output.

Formatting + placement requirements (must stay baseline-diff-stable):
- Preserve registration order for attributes.
- Preserve authored order for attribute argument values.
- Use `ISerializableType::Serialize` to format argument values.
- Placement rules relative to existing decorators:
  - Type-level:
    - keep existing `@FullName`, `@ValueType`, `@Serializable` lines first (as today)
    - then print all **type attributes** (`@Attribute:...`) in registration order
    - then print the type declaration (`class`/`interface`/`struct`/`enum`/`primitive ...`)
  - Member-level (property/event/method/constructor/field):
    - print **member attributes** (`    @Attribute:...`) first
    - then print existing C++ template decorators if any (`@ReferenceTemplate`, `@AttachTemplate`, `@DetachTemplate`, `@InvokeTemplate`, `@ClosureTemplate`)
    - then print the member declaration line/block
  - Parameter-level:
    - print parameter attributes as `    @ParamAttribute:<ParamName>:...` lines **immediately before** the owning method/constructor signature line (after member-attribute lines, before the signature).

Example text shapes:

```
@Attribute:<AttrType>(<ArgType>:<Data>, <ArgType>:<Data>)
@ParamAttribute:x:<AttrType>(<ArgType>:<Data>)
    function Foo(x : vint, y : WString) : void;
```


## STEP 5: Add cross-project tests for runtime + metaonly attribute behavior

### Files to add (shared)
- Test\Source\TestReflection_Attribute.h
- Test\Source\TestReflection_Attribute.cpp

Shared file content requirements:
- Define an attribute struct and its reflection (only when not metaonly):

```
struct MyAttribute
{
	WString name;
	vint number;
};

#if !defined(VCZH_DEBUG_NO_REFLECTION) && !defined(VCZH_DEBUG_METAONLY_REFLECTION)
BEGIN_STRUCT_MEMBER(MyAttribute)
	STRUCT_MEMBER(name)
	STRUCT_MEMBER(number)
END_STRUCT_MEMBER(MyAttribute)
#endif
```

- Define a test class exercising:
  - `ATTRIBUTE_TYPE` on the type
  - `ATTRIBUTE_MEMBER` on property/event/method/ctor/field
  - `ATTRIBUTE_PARAMETER` on at least one method parameter
  - multiple attributes per target and at least one 0-arg attribute usage
- Provide:
  - a type loader `TestTypeLoader_Attribute : Object, ITypeLoader` that registers these test types (follow the established `TYPE_LIST(ADD_TYPE_INFO)` pattern).
  - one exported function `TestReflectionAttributes()` that resolves by type name (metaonly-safe) and validates all attributes using `TEST_ASSERT` only.

### Wire into metadata round-trip tests (IMPORTANT: put TEST_CASE in Test\Source)
The `Metadata_Generate` and `Metadata_Test` projects already compile:
- `Test\Source\TestMetadataGeneration.cpp`
- `Test\Source\TestMetadata.cpp`

Update them to define the test cases there (not in the driver/entry files):

- In `Test\Source\TestMetadataGeneration.cpp`:
  - Add a `TEST_CASE` that:
    - loads predefined types
    - registers the custom loader for the attribute test types **before** calling `GetGlobalTypeManager()->Load()` (use `TEST_CASE_REFLECTION_LOADER` pattern from `Test\Source\Common.h`)
    - calls `TestReflectionAttributes()`
    - writes `.bin` via `GenerateMetaonlyTypes(...)`
    - writes `.txt` via `LogTypeManager(...)`
    - ensures `ResetGlobalTypeManager()` is executed (the `TEST_CASE_REFLECTION_*` pattern already guarantees this)

- In `Test\Source\TestMetadata.cpp`:
  - Add a `TEST_CASE` that runs the metaonly round-trip:
    - loads metaonly descriptors from `.bin` (same logic currently in `Test\UnitTest\Metadata_Test\LoadMetadata.cpp`)
    - calls `TestReflectionAttributes()` (no custom loader required here; types come from the loaded `.bin`)
    - writes `[2].txt` via `LogTypeManager(...)`
    - compares baseline text and new output
    - must call `ResetGlobalTypeManager()` on both success and failure (wrap in `try/catch` like `TEST_CASE_REFLECTION_BODY` does).

Driver/entry files cleanup:
- Remove `TEST_FILE` / `TEST_CASE` blocks from:
  - `Test\UnitTest\Metadata_Generate\GenerateMetadata.cpp`
  - `Test\UnitTest\Metadata_Test\LoadMetadata.cpp`
  leaving them as thin drivers only (or empty translation units if nothing else is needed).

## STEP 6: Verification sequence (per AFFECTED PROJECTS)
1) Build `REPO-ROOT\Test\UnitTest` with Debug|Win32.
2) Run `Metadata_Generate` with Debug|Win32 (regenerates Reflection32.*).
3) Build `REPO-ROOT\Test\UnitTest` with Debug|x64.
4) Run `Metadata_Generate` with Debug|x64 (regenerates Reflection64.*).
5) Run `Metadata_Test` with Debug|x64 (validates round-trip against baseline).
6) Run `Metadata_Test` with Debug|Win32 (extra coverage required by the task).
7) Run `VlppReflection` with Debug|x64.

# FIXING ATTEMPTS

# !!!FINISHED!!!

