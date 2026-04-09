# !!!PLANNING!!!

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

### What to change

Target files:
- `Source\Reflection\Metadata\Metadata.h`
- `Source\Reflection\Metadata\Metadata.cpp`

1) Implement a concrete `IAttributeInfo` implementation stored by type descriptors.

Add a class (location can be `Metadata.h` near other impls, with method bodies in `Metadata.cpp`):

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

2) Make `AttributeBagSource` dispatch points usable (no more `CHECK_FAIL`).

In `Source\Reflection\Metadata\Metadata.h` today:
- `AttributeBagSource::GetAttributeCountInternal` and `GetAttributeInternal` are **non-virtual** and call `CHECK_FAIL(L"Not Implemented!")`.
- `MemberInfoBase<TMemberInfo>` always calls these dispatch points.

Change them to `virtual`, with safe defaults:

```
class AttributeBagSource : public Object
{
	...
protected:
	virtual vint GetAttributeCountInternal(IMemberInfo* memberInfo)
	{
		return 0;
	}
	virtual IAttributeInfo* GetAttributeInternal(IMemberInfo* memberInfo, vint index)
	{
		return nullptr;
	}
};
```

3) Centralize attribute storage per `ITypeDescriptor` (as required).

Add storage and public registration APIs to `TypeDescriptorImplBase` (or `TypeDescriptorImpl` if you prefer keeping non-virtual state out of the base) so that:
- `memberInfo == nullptr` means the bag is the `ITypeDescriptor` itself.
- otherwise, the bag is a member/parameter.

Required storage shape (avoid accidental O(n^2) behavior from re-filtering a flat list on every `GetAttribute(i)` call):

```
collections::List<Ptr<IAttributeInfo>>							typeAttributes;
collections::Group<IMemberInfo*, Ptr<IAttributeInfo>>		memberAttributes;

IMemberInfo*	lastRegisteredMember = nullptr;
IMethodInfo*	lastRegisteredMethod = nullptr;
```

Notes:
- `collections::Group<K, V>` is preferred here (keyed 1-to-many) so per-member attribute iteration stays in authored order while avoiding repeated full scans.
- If `Group` turns out to be unsuitable for any reason, use an equivalent keyed 1-to-many container that keeps insertion order per key.

and override dispatch points in `TypeDescriptorImplBase`:

```
vint GetAttributeCountInternal(IMemberInfo* memberInfo)override;
IAttributeInfo* GetAttributeInternal(IMemberInfo* memberInfo, vint index)override;
```

Dispatch behavior:
- If `memberInfo == nullptr`:
  - count/return from `typeAttributes`.
- Else:
  - count/return from `memberAttributes` for `memberInfo`, preserving registration order for that specific member.

4) Provide registration helpers on `TypeDescriptorImplBase`:

```
void RegisterTypeAttribute(Ptr<IAttributeInfo> info);
void RegisterMemberAttribute(IMemberInfo* memberInfo, Ptr<IAttributeInfo> info);

IMemberInfo* GetLastRegisteredMember()const;
IMethodInfo* GetLastRegisteredMethod()const;

void SetLastRegisteredMember(IMemberInfo* member); // also clears lastRegisteredMethod
void SetLastRegisteredMethod(IMethodInfo* method); // also updates lastRegisteredMember = method
void ClearLastRegisteredMethod();
```

Hard contract (to make macro binding predictable):
- Registering a non-method member (property/event/field) must set `lastRegisteredMember` and clear `lastRegisteredMethod`.
- Registering a method/constructor must set both `lastRegisteredMember` and `lastRegisteredMethod`.

5) Update existing member-registration helpers to maintain “last registered” tracking.

In `TypeDescriptorImpl` (`Metadata.cpp` around `AddProperty/AddEvent/AddMethod/AddConstructor`):
- After `AddMethod` / `AddConstructor`: set both lastRegisteredMember and lastRegisteredMethod to the returned method.
- After `AddProperty` / `AddEvent`: set lastRegisteredMember to the returned member, and **clear** lastRegisteredMethod.
  - This prevents `ATTRIBUTE_PARAMETER` from accidentally binding to a previously-added method used to implement properties.

### Why
- `IAttributeBag` already exists and member info implementations already route through `AttributeBagSource`; the current stubs crash.
- Centralized storage per descriptor is required by the task: every bag delegates to the owner type descriptor.
- “Last registered member/method” is needed to implement `ATTRIBUTE_MEMBER` / `ATTRIBUTE_PARAMETER` without rewriting all existing `CLASS_MEMBER_*` macros.

---

## STEP 2: Implement macro authoring API (ATTRIBUTE_TYPE / ATTRIBUTE_MEMBER / ATTRIBUTE_PARAMETER)

### What to change

Target file:
- `Source\Reflection\Reflection\Macros.h`

1) Add helper template(s) (preferably in a small `namespace` block in `Macros.h` or a nearby header) to build `Ptr<IAttributeInfo>` with validation:

Key requirements enforced:
- `TYPE{ARG1, ...}` must compile.
- Attribute type must be a reflectable struct (`TypeDescriptorFlags::Struct`).
- Each argument must be a serializable primitive: `Value(arg).GetTypeDescriptor()->GetSerializableType() != nullptr`.

Sketch:

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

2) Define the 3 macros in `Macros.h` so they are usable inside `LoadInternal()` generated by `BEGIN_*_MEMBER`.

Behavior:
- `ATTRIBUTE_TYPE(TYPE, ...)`:
  - `this->RegisterTypeAttribute(MakeAttributeInfo<TYPE>(...))`
- `ATTRIBUTE_MEMBER(TYPE, ...)`:
  - `auto member = this->GetLastRegisteredMember(); CHECK_ERROR(member != nullptr, L"... must appear after a member.");`
  - `this->RegisterMemberAttribute(member, MakeAttributeInfo<TYPE>(...))`
- `ATTRIBUTE_PARAMETER(PARAMETER_NAME, TYPE, ...)`:
  - `auto method = this->GetLastRegisteredMethod(); CHECK_ERROR(method != nullptr, L"... must appear after a method.");`
  - find parameter by name in `method->GetParameter(i)->GetName()`; on missing -> `CHECK_ERROR(false, ...)`
  - `this->RegisterMemberAttribute(parameterInfo, MakeAttributeInfo<TYPE>(...))`

Notes:
- Use `__VA_ARGS__` directly (0 args allowed: `TYPE{}` is valid and should pass the `requires` check).
- Accept wide string literals (e.g. `L"name"`) by boxing them as `vl::WString` (matches the task example).
- `CLASS_MEMBER_FIELD` should be covered by the same “non-method member” last-member tracking path used by property/field registration (confirm the exact macro expansion during implementation).
- Use `CHECK_ERROR` messages that include macro name and usage rule, because misplacement is a common authoring mistake.
- Attribute validation must be enabled in both full and metaonly reflection builds (gate only on `VCZH_DEBUG_NO_REFLECTION`).

3) Support `ATTRIBUTE_MEMBER` after `STRUCT_MEMBER` (required).

Today `STRUCT_MEMBER(FIELDNAME)` directly mutates `fields.Add(...)` in the struct descriptor (`Macros.h` around line ~258).
To make last-member tracking work for struct fields, introduce an `AddField(...)` helper on `StructTypeDescriptor` and change the macro to call it:

- In `Source\Reflection\Metadata\Metadata.h` (inside `StructTypeDescriptor`):
  - add `IPropertyInfo* AddField(Ptr<IPropertyInfo> value)` that inserts into `fields`, sets `lastRegisteredMember`, clears `lastRegisteredMethod`.
  - this makes struct fields behave consistently with properties/events for last-member tracking.
- In `Macros.h`:

```
#define STRUCT_MEMBER(FIELDNAME) \
	AddField(Ptr(new StructFieldInfo<decltype(((StructType*)0)->FIELDNAME)>(this, &StructType::FIELDNAME, L ## #FIELDNAME)));
```

### Why
- The task explicitly requires macro-based authoring compatible with existing `BEGIN_CLASS_MEMBER/END_CLASS_MEMBER` patterns.
- Centralizing to descriptor-level storage avoids per-member custom logic and matches the already-existing `MemberInfoBase` delegation.

---

## STEP 3: Extend metaonly metadata schema to serialize/deserialize attributes

### What to change

Target file:
- `Source\Reflection\DescriptableInterfaces_Metaonly.cpp`

1) Add new metadata structs near existing ones (around `ParameterInfoMetadata/MethodInfoMetadata/...`):

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

2) Add attribute lists to existing metadata structs:
- `TypeDescriptorMetadata`:
  - `List<AttributeInfoMetadata> attributes;`
- `MethodInfoMetadata` / `PropertyInfoMetadata` / `EventInfoMetadata` / `ParameterInfoMetadata`:
  - `List<AttributeInfoMetadata> attributes;`

3) Update serialization definitions (the `BEGIN_SERIALIZATION(...)` section) to include these new fields.

4) Update generation (`GenerateMetaonlyTypeDescriptor/MethodInfo/PropertyInfo/EventInfo`):

For each bag (`td`, `mi`, `pi`, `ei`, and each `parameter`):
- iterate `GetAttributeCount()`.
- build `AttributeInfoMetadata`:
  - `attributeTypeName = info->GetAttributeType()->GetTypeName()`
  - for each attribute value:
    - `Value v = info->GetAttributeValue(i)`
    - `auto vtd = v.GetTypeDescriptor(); CHECK_ERROR(vtd != nullptr, ...)`
    - `auto st = vtd->GetSerializableType(); CHECK_ERROR(st != nullptr, ...)`
    - `WString data; CHECK_ERROR(st->Serialize(v, data), ...)`
    - append `{ vtd->GetTypeName(), data }`

5) Update loading (`LoadMetaonlyTypes`):

After reading all `TypeDescriptorMetadata/MethodInfoMetadata/PropertyInfoMetadata/EventInfoMetadata`:
- construct `AttributeInfoImpl` objects for each `AttributeInfoMetadata` and register them onto:
  - the owning type descriptor (type-level)
  - the owning member/parameter (member-level)

Implementation detail (recommended):
- Add a helper `LoadAttributes(MetaonlyReaderContext*, AttributeBagSource*, IMemberInfo* memberOrNull, const List<AttributeInfoMetadata>&)` to:
  - resolve attribute type by name (`GetTypeDescriptor(attributeTypeName)` should work after loader installs all descriptors; if not yet installed, resolve by scanning `context->tds` by `metadata->typeName`).
  - resolve each value using `context->serializableTypes[valueTypeName]->Deserialize(data, v)`.
  - register using the same `RegisterTypeAttribute/RegisterMemberAttribute` APIs as runtime.

Binary compatibility:
- This is an intentional schema change.
- Older `Reflection*.bin` produced before this change are expected to fail to load (regeneration is required).
- `Metadata_Test` consumes `.bin` freshly generated by `Metadata_Generate`, so updating both together is acceptable.

### Why
- Without extending metaonly schema, attributes cannot round-trip through `GenerateMetaonlyTypes/LoadMetaonlyTypes`.
- Using `ISerializableType` ensures only primitive serializable values are supported, matching the task requirement.

---

## STEP 4: Update LogTypeManager text output to include attributes

### What to change

Target file:
- `Source\Reflection\DescriptableInterfaces_Log.cpp`

Add deterministic printing of attributes for:
- type descriptors (`ITypeDescriptor`)
- properties/events/methods/constructors (`IMemberInfo`)
- method parameters (`IParameterInfo`)

Hard requirements:
- Attribute list order must be registration order for each bag.
- Attribute argument order must be authored order.
- If a bag has zero attributes, print nothing (no blank marker lines).

Recommended text shape (stable + easy to diff):
- Type attributes printed immediately after `@Serializable/@ValueType` lines and before the `class/interface/...` declaration body.
- Member attributes printed immediately before the member they decorate.
- Parameter attributes printed immediately before the method signature, but MUST include a stable parameter identity (at least parameter name; also include index if desired) so diffs cannot confuse them with method-level attributes.

Example lines:

```
@Attribute:<AttrType>(<ArgType>:<Data>, <ArgType>:<Data>)
@ParamAttribute:x:<AttrType>(<ArgType>:<Data>)
    function Foo(x : vint, y : WString) : void;
```

For argument formatting, reuse the same serialization rule:
- for each `Value` argument, use `valueTd->GetSerializableType()->Serialize(value, data)`.

### Why
- `Metadata_Test` compares `Reflection*.txt` baseline and `[2].txt`; attributes must be represented to validate round-trip.

---

## STEP 5: Add cross-project tests for runtime + metaonly attribute behavior

### What to change

Target files:
- New shared files (must be added to both test projects):
  - `Test\Source\TestReflection_Attribute.h`
  - `Test\Source\TestReflection_Attribute.cpp`
- Update metadata tests:
  - `Test\UnitTest\Metadata_Generate\GenerateMetadata.cpp`
  - `Test\UnitTest\Metadata_Test\LoadMetadata.cpp`

Shared test file responsibilities:
1) Define one (or two) attribute struct(s) and their reflection (only when not metaonly):
   - include at least one zero-argument attribute usage (`ATTRIBUTE_*(MyAttr)` with no args).
   - include multiple attributes applied to the same target to validate ordering.

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

2) Define one test class with all supported targets (type + property/event/method/ctor + parameter), and at least one reflectable struct with fields.
3) Author attributes using the new macros in the reflection definition (only in non-metaonly builds), including:
   - type-level attributes
   - member-level attributes
   - parameter-level attributes
   - struct-field attributes (via `STRUCT_MEMBER(...)` followed by `ATTRIBUTE_MEMBER(...)`)
4) Provide a single exported function (declared in `.h`, implemented in `.cpp`) that validates:
- the type has expected attributes
- each member has expected attributes
- each parameter has expected attributes
- struct field attributes are queryable
- attribute order and value order are preserved
- values survive metaonly binary round-trip (implicitly verified by `.txt` vs `[2].txt`)

Implementation notes for the test function:
- Always resolve by type name so it works under metaonly (no associated descriptor):
  - `auto td = GetTypeDescriptor(TypeInfo<TestClass>::content.typeName); TEST_ASSERT(td != nullptr);`
- Use `TEST_ASSERT` only (no `TEST_FILE/TEST_CASE` in the shared file).

Wire-in test cases:
- In `Metadata_Generate\GenerateMetadata.cpp`:
  - inside the existing `TEST_CASE`, call `TestReflectionAttributes();` after `LoadPredefinedTypesForTestCase()` and before writing `.bin/.txt`.
- In `Metadata_Test\LoadMetadata.cpp`:
  - inside the existing `TEST_CASE`, call `TestReflectionAttributes();` after `LoadPredefinedTypesForTestCase()` and before writing `[2].txt`.

Project file integration:
- Add the shared `.h/.cpp` to both `Metadata_Generate.vcxproj` and `Metadata_Test.vcxproj` (and `.filters`) following `SourceFileManagement.md`.
- Linux impact: if these shared test files are part of any Linux unit test build, update the corresponding `Test\Linux` `vmake` wiring accordingly.

### Why
- This provides coverage for both:
  - runtime reflection authoring via macros
  - metaonly serialization round-trip via `.bin` loading

---

# TEST PLAN

## Attribute macro authoring
1) `ATTRIBUTE_TYPE` attaches to `ITypeDescriptor` and is visible via `td->GetAttributeCount()/GetAttribute(i)`.
2) `ATTRIBUTE_MEMBER` attaches to the immediately preceding member (property/event/method/ctor/field).
3) `ATTRIBUTE_PARAMETER` attaches to the named parameter of the immediately preceding method/ctor.
4) Misuse cases (should raise `Error` / `CHECK_ERROR`):
   - `ATTRIBUTE_MEMBER` before first member.
   - `ATTRIBUTE_PARAMETER` when last registered item is not a method/ctor.
   - `ATTRIBUTE_PARAMETER` referencing a non-existing parameter name.
   - Prefer expressing these as dedicated unit tests using existing helpers like `TEST_ERROR(...)` / `TEST_EXCEPTION(...)` (do not embed expected-failure authoring into metadata generation without an error-catching harness).

## Attribute payload validation
5) Attribute type must be a reflectable struct:
   - Using a non-struct reflectable type must fail at load time with a clear error.
6) Each argument must be serializable:
   - Using a non-serializable type (e.g. a class pointer / non-serializable struct) must raise `Error`.
7) Ordering:
   - Multiple attributes on the same target preserve registration order.
   - Multiple values in one attribute preserve authored order.

## Metaonly round-trip
8) Run `Metadata_Generate` (Win32 + x64) to generate `Reflection{32,64}.bin/.txt` and ensure attribute lines appear in the logged `.txt`.
9) Run `Metadata_Test` (x64 required; Win32 extra coverage) to load `.bin` and ensure `Reflection{32,64}[2].txt` matches baseline.
   - If schema change causes baseline drift, re-run `Metadata_Generate` to overwrite baseline `.txt` then rerun `Metadata_Test`.

## Full unit test
10) Run `VlppReflection` unit tests (Debug|x64) to ensure no regressions.

# !!!FINISHED!!!

