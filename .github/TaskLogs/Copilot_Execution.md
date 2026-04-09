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

## STEP 1: Implement runtime attribute storage (non-metaonly) [DONE]

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

## STEP 2: Implement macro authoring API (ATTRIBUTE_TYPE / ATTRIBUTE_MEMBER / ATTRIBUTE_PARAMETER) [DONE]

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


## STEP 3: Extend metaonly metadata schema to serialize/deserialize attributes [DONE]

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


## STEP 4: Update LogTypeManager text output to include attributes [DONE]

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


## STEP 5: Add cross-project tests for runtime + metaonly attribute behavior [DONE]

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

## STEP 6: Verification sequence (per AFFECTED PROJECTS) [DONE]
1) Build `REPO-ROOT\Test\UnitTest` with Debug|Win32.
2) Run `Metadata_Generate` with Debug|Win32 (regenerates Reflection32.*).
3) Build `REPO-ROOT\Test\UnitTest` with Debug|x64.
4) Run `Metadata_Generate` with Debug|x64 (regenerates Reflection64.*).
5) Run `Metadata_Test` with Debug|x64 (validates round-trip against baseline).
6) Run `Metadata_Test` with Debug|Win32 (extra coverage required by the task).
7) Run `VlppReflection` with Debug|x64.

# FIXING ATTEMPTS

## Fixing attempt No.1
- why the previous change failed
  - `AttributeBagSource` used `collections::Group::GetByKey`, but `Group` exposes `Get` / `operator[]` instead, so all three unit-test projects failed to compile.
  - `ATTRIBUTE_PARAMETER` built dynamic `CHECK_ERROR` messages as `WString`, but `CHECK_ERROR` requires `const wchar_t*`, causing macro compilation failures.
- what you changed
  - Replaced `GetByKey` usage with `Contains` + `Get` in `Source\Reflection\Metadata\Metadata.h`, and kept the helper behavior safe by returning `0` / `nullptr` when no attributes are registered or an index is out of range.
  - Converted the dynamic `ATTRIBUTE_PARAMETER` error messages in `Source\Reflection\Reflection\Macros.h` to `.Buffer()` so they match `CHECK_ERROR`'s expected argument type.
- why it should work
  - The code now matches the actual Vlpp collection API and preserves the intended safe default semantics for attribute bags.
  - The macro still emits the same diagnostic text, but now passes a `const wchar_t*`, which `CHECK_ERROR` can compile.

## Fixing attempt No.2
- why the previous change failed
  - The new shared attribute test file still used raw comma-separated parameter names in `CLASS_MEMBER_METHOD`, so the macro parser treated them as extra macro arguments and broke the translation unit.
  - `TestMetadataGeneration.cpp` and `TestMetadata.cpp` called the helper APIs without qualifying the `reflection_test_attribute` namespace.
  - The metaonly attribute metadata used `List<T>` for nested payload structs; Vlpp lists require copyable elements during internal buffer growth, so nested `List` members made those structs non-copyable and triggered `C2280`.
- what you changed
  - Added the repo-standard `_` comma helper in `Test\Source\TestReflection_Attribute.cpp` and used `{L"x" _ L"y"}` for the method parameter list.
  - Imported `reflection_test_attribute` in the metadata test source files so the shared helper declarations are visible.
  - Switched nested attribute metadata containers in `Source\Reflection\DescriptableInterfaces_Metaonly.cpp` to `List<Ptr<...>>` and updated generation/loading code to populate and read pointer-backed metadata records.
- why it should work
  - The registration macros now receive the exact argument shape they expect.
  - The shared test helpers are resolved from the correct namespace in both driver test files.
  - Pointer-backed metadata records match the file's existing serialization style for non-copyable payload types and avoid list reallocation copying deleted copy constructors.

## Fixing attempt No.3
- why the previous change failed
  - Boxing `MyAttribute` in the new attribute tests instantiated `TypedBox<T>::ComparePrimitive`, which expects the boxed value type to support `<=>`.
  - The new test-only structs had no spaceship operator, so both metadata projects failed while compiling boxing support for `MyAttribute`.
- what you changed
  - Added defaulted `operator<=>` implementations to `MyAttribute`, `EmptyAttribute`, and `AttributeRecord` in `Test\Source\TestReflection_Attribute.cpp`.
- why it should work
  - These structs are pure value types used in reflection metadata tests, so defaulted three-way comparison is appropriate and gives `TypedBox<T>` the comparison support it requires without changing the intended test behavior.

## Fixing attempt No.4
- why the previous change failed
  - The shared attribute tests passed raw integer literals to `ATTRIBUTE_*`, so the boxed values were `system::Int32`.
  - `TestReflectionAttributes()` intentionally validates those numeric arguments as `vint`; that only matches on Win32, so the x64 `Metadata_Generate` run failed with `Argument "value" cannot convert from "system::Int32" to "system::Int64"`.
- what you changed
  - Updated every numeric attribute argument in `Test\Source\TestReflection_Attribute.cpp` from raw integer literals to explicit `vint(...)` values.
- why it should work
  - The test attributes now use the repo's preferred integer type directly, so both the runtime assertions and the serialized metadata schema are consistent with the platform-specific `vint` reflection type on Win32 and x64.

## Fixing attempt No.4
- why the previous change failed
  - Runtime type attributes were registered inside descriptor `LoadInternal()`, but `ITypeDescriptor::GetAttributeCount()` and `GetAttribute()` did not trigger lazy descriptor loading first.
  - The new verification test queried type-level attributes before any other API had forced the descriptor to load, so the type attribute bag was still empty.
- what you changed
  - Added a `LoadForAttributeAccess()` hook to `TypeDescriptorImplBase` and overrode it in both lazy descriptor implementations (`ValueTypeDescriptorBase` and `TypeDescriptorImpl`) so type attribute queries force descriptor loading before reading the bag.
- why it should work
  - Type-level attributes are authored during `LoadInternal()`, so forcing the same lazy-load path for `GetAttributeCount()` / `GetAttribute()` makes type attribute access consistent with the rest of the descriptor API.

## Fixing attempt No.5
- why the previous change failed
  - The earlier integer-literal fix updated the class-level attribute sites but missed the struct-field attribute in `AttributeRecord`.
  - That remaining raw `91` still boxed as `system::Int32`, so the x64 metadata-generation verification continued to fail when the test validated it as `vint`.
- what you changed
  - Changed the remaining `ATTRIBUTE_MEMBER(MyAttribute, L"struct-field", 91)` call in `Test\Source\TestReflection_Attribute.cpp` to pass `vint(91)`.
- why it should work
  - All numeric attribute payloads for `MyAttribute` now use `vint`, so runtime validation and serialized metadata use the same reflected integer type on both Win32 and x64.

## Fixing attempt No.6
- why the previous change failed
  - The attribute system stores the boxed argument values exactly as authored, instead of normalizing them to the destination struct field type.
  - The shared verification helper still assumed every numeric attribute payload was boxed as `vint`, but x64 can legitimately encounter `system::Int32` payloads, so the test itself was over-constrained.
- what you changed
  - Added a small helper in `Test\Source\TestReflection_Attribute.cpp` to read signed attribute integers from either `vint32_t` or `vint64_t`, and changed `AssertMyAttribute` to compare through that helper.
- why it should work
  - The verification now checks the actual numeric payload value while still asserting that the argument is a reflected signed integer, which matches the attribute storage semantics on both Win32 and x64.

## Fixing attempt No.6
- why the previous change failed
  - The attribute authoring helpers still boxed ordinary integral arguments using their native C++ literal types.
  - That makes reflection metadata fragile across architectures because plain integer literals become `system::Int32`, while repo conventions and many tests expect `vint`.
- what you changed
  - Updated `detail::attribute_macro::BoxAttributeArgument` in `Source\Reflection\Reflection\Macros.h` to normalize integral attribute arguments (except `bool` and `wchar_t`) to `vint` before boxing.
- why it should work
  - The `ATTRIBUTE_*` macros now treat natural integer literals consistently with the rest of the codebase, avoiding platform-specific reflected integer mismatches even when callers do not manually wrap values in `vint(...)`.

## Fixing attempt No.7
- why the previous change failed
  - Metaonly attribute values were deserialized while loading the `.bin` payload, before the global type manager had loaded the corresponding reflected types.
  - `SerializableType<T>::Deserialize` therefore produced boxed `Value` objects without a usable `ITypeDescriptor*`, which later broke `LogTypeManager` when it tried to serialize attribute arguments back to text in `Metadata_Test`.
- what you changed
  - In `Source\Reflection\DescriptableInterfaces_Metaonly.cpp`, after deserializing each attribute argument I now resolve its reflected type name from the loaded type-descriptor table and rewrap the boxed value with `Value::From(value.GetBoxedValue(), reflectedValueType)`.
- why it should work
  - The deserialized payload keeps the same boxed data, but now carries the correct reflected type descriptor from the loaded metaonly type set, so attribute logging and any later reflection-based inspection can resolve the argument type reliably.

# !!!FINISHED!!!

# !!!VERIFIED!!!

