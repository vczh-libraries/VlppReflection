# !!!TASK!!!

# PROBLEM DESCRIPTION
You are going to add the following macros to enrich the list of macro-based reflection.
- ATTRIBUTE_TYPE(TYPE, ARG1, ...)
  - When it appears inside a type reflection definition, e.g. between BEGIN_CLASS_MEMBER and END_CLASS_MEMBER and all possible other choices, it addes an attribute to then type. Zero to multiple are allowed
- ATTRIBUTE_MEMBER(TYPE, ARG1, ...)
  - When it appears after any possible members, it adds an attribute to the member. Zero to multiple are allowed. It is not allowed when appearing before the first member.
- ATTRIBUTE_PARAMETER(PARAMETER-NAME, TYPE, ARG1, ...)
  - When it appears after any possible method, it adds an attribute to the argument with that name. Zero to multiple are allowed, parameter name should exists.

When saying `TYPE, ARG1, ARG2, ...` it means that:
- TYPE{ARG1, ARG2, ...} should compile, you can leverage type_traits to implement it. If it does not compile, the C++ code should not compile too.
- It creates an `IAttributeInfo` object, the GetAttributeType returns reflection::description::GetTypeDescriptor<TYPE>();, each ARG becomes a `Value`. Only serializable primitive types are allowed, you need to box the value, get its `ITypeDescriptor` and see if it is serializable. If not, an `Error` should raise.
- All `IAttributeBag` types redirect its calls to the owner type descriptor's `AttributeBagSource` base class. Attributes are centralized for each `ITypeDescriptor`. You can add public methods to AttributeBagSource to register new attributes. In this class, a null `methodInfo` means such `IAttributeBag` is the `ITypeDescriptor` itself, otherwise it is for a member.
- The attribute type must be a reflectable struct

Here I give you an example, if we have an attribute called `MyAttribute`:
struct MyAttribute
{
  WString name;
  vint number;
};

You can use the macro like this
BEGIN_CLASS_MEMBER(Anything)
  ATTRIBUTE_TYPE(MyAttribute, L"name", 100)
END_CLASS_MEMBER(Anything)
and such attribute is attached to the type.

In order to implement it, you should:
- Implement these macros
- Implement metadata serialization and deserialization
- Prepare a shared header/cpp file under REPO-ROOT\Test\Source, to define the type and its reflection. You need to follow the pattern so that `VCZH_DEBUG_NO_REFLECTION` and `VCZH_DEBUG_METAONLY_REFLECTION` all works.
  - In this file, there will be a test class with test attributes (struct), all possible members have attributes, including the type itself. Register that class.
  - In this file, there will be a function (declared in header and implemented in cpp), it will be a test case to assert that all attributes can be retrieved from the `ITypeDescriptor` of the test class and all its members. You must use `TEST_ASSERT` for assertion but no `TEST_FILE` or `TEST_CASE` is needed.
  - Unfortunately no test case is covering this, what you need to do is:
    - Use the shared file you created in both Metadata_Generate and Metadata_Test, so that the generated metadata file and logged text file have the test class.
    - In Metadata_Generate's `TestMetadataGeneration.cpp`, make a test case to call that test function in the shared file.
    - In Metadata_Test's `TestMetadata.cpp`, make a test case to call that test function in the shared file after loading the meatdata. You can check out `LoadMetadata.cpp` for how to load the metadata generated from `Metadata_Generate`.
    - These test files should reset the type loader just like other test files so that they won't affect other test cases in these two projects.
- Both Meatadata_Generate and Metadata_Test should be executed in debug with both x64 and Win32.
- You need to figure out why Metadata_Generate load predefined types but Metadata_Test doesn't (so that it can load `ITypeDescriptor` instances from metadata generated from `Metadata_Generate`) to have a better knowledge for test case authorizing.

# UPDATES

# INSIGHTS AND REASONING
## Current state / evidence
- `vl::reflection::description::IAttributeInfo` and `IAttributeBag` are already defined (`Source\Reflection\DescriptableInterfaces.h` around the `GetAttributeType / GetAttributeValueCount / GetAttributeValue` methods), but there is no working attribute storage/implementation.
- `AttributeBagSource` exists in `Source\Reflection\Metadata\Metadata.h` but its internal dispatch points are stubbed with `CHECK_FAIL(L"Not Implemented!")` (`GetAttributeCountInternal` / `GetAttributeInternal`). Calls from `MemberInfoBase<TMemberInfo>` route through this base class.
- `MethodGroupInfoImpl::GetAttributeCount/GetAttribute` currently return `0/nullptr` (`Source\Reflection\Metadata\Metadata.cpp` around lines ~609-617), and metaonly counterparts do the same (`Source\Reflection\DescriptableInterfaces_Metaonly.cpp` around lines ~469-479).
- Metaonly metadata structs (`TypeDescriptorMetadata`, `MethodInfoMetadata`, `PropertyInfoMetadata`, `EventInfoMetadata`, `ParameterInfoMetadata`) do not contain any attribute fields (`Source\Reflection\DescriptableInterfaces_Metaonly.cpp` around lines ~152-218). Therefore attributes cannot round-trip via `GenerateMetaonlyTypes` / `LoadMetaonlyTypes` today.

## Design goals
1. Provide macro-based authoring (`ATTRIBUTE_TYPE`, `ATTRIBUTE_MEMBER`, `ATTRIBUTE_PARAMETER`) that integrates with existing macro registration patterns (inside `LoadInternal()` generated by `BEGIN_*_MEMBER`).
2. Centralize attribute storage per `ITypeDescriptor` as required: all `IAttributeBag` implementations forward to the owner type descriptor’s `AttributeBagSource`.
3. Ensure attribute arguments are limited to serializable primitive types by checking `Value`’s `ITypeDescriptor::GetSerializableType()` (and using `ISerializableType` to serialize into metadata).
4. Ensure binary metadata round-trip correctness: attributes must be generated into `.bin`, logged into `.txt`, and reloaded by `Metadata_Test` with identical `.txt` output.

## Proposed architecture
### 1) Runtime (non-metaonly) attribute model
- Add a concrete `AttributeInfoImpl : Object, IAttributeInfo` implementation stored by type descriptors.
  - Fields: `ITypeDescriptor* attributeType; List<Value> values;`.
  - `GetAttributeType()` returns the stored descriptor.
  - `GetAttributeValueCount()/GetAttributeValue(i)` enumerate stored boxed values.

### 2) Centralized storage in `AttributeBagSource`
- Keep the requirement “all `IAttributeBag` calls redirect to the owner type descriptor” by making the owner type descriptor (`TypeDescriptorImplBase` / `MetaonlyTypeDescriptor`) the only place that actually stores attributes.
- Use an explicit storage shape (and document invariants):
  - Type-level attributes: `collections::List<Ptr<IAttributeInfo>>`.
  - Member/parameter-level attributes: `collections::Group<IMemberInfo*, Ptr<IAttributeInfo>>` keyed by the exact `IMemberInfo*` instance.
    - This includes `IParameterInfo*` because `IParameterInfo` is-a `IMemberInfo` and `MemberInfoBase<IParameterInfo>` already routes into `AttributeBagSource` with `this`.
- Ordering invariants (must hold in runtime and metaonly):
  - Attributes for a given target are kept in registration order.
  - Attribute argument values are kept in authored order (`GetAttributeValue(i)` must round-trip deterministically).
- Add public registration APIs on `AttributeBagSource` (names TBD):
  - `RegisterTypeAttribute(Ptr<IAttributeInfo>)`.
  - `RegisterMemberAttribute(IMemberInfo* member, Ptr<IAttributeInfo>)`.
- Fix the existing dispatch points so `MemberInfoBase::GetAttributeCount/GetAttribute` work for all `IAttributeBag` implementors:
  - Change `GetAttributeCountInternal` / `GetAttributeInternal` to `virtual`.
  - Base-class behavior must be safe defaults (`0` / `nullptr`), not `CHECK_FAIL`, so descriptors without attributes behave normally.
  - Only attribute-owning descriptors (`TypeDescriptorImplBase` at runtime and `MetaonlyTypeDescriptor` in metaonly) override to look up from the two containers above.

### 3) Macro layer (`ATTRIBUTE_*`) integration
`ATTRIBUTE_MEMBER` and `ATTRIBUTE_PARAMETER` need to attach to “the most recently registered member/method”, but doing this via hidden locals inside `LoadInternal()` would require invasive edits across all `CLASS_MEMBER_*` macro variants.

Instead, keep blast radius small and align with existing patterns by tracking “last registered member/method” as `protected` fields in the descriptor implementation hierarchy (e.g., in `AttributeBagSource` or `TypeDescriptorImplBase`):
- `IMemberInfo* lastRegisteredMember = nullptr;`
- `IMethodInfo* lastRegisteredMethod = nullptr;`

Update these tracking fields inside existing registration helpers (the exact helper list depends on where member objects are created today):
- `AddMethod(...)` sets both `lastRegisteredMember` and `lastRegisteredMethod`.
- `AddProperty(...)`, `AddEvent(...)` set `lastRegisteredMember` and clear `lastRegisteredMethod`.
- `AddConstructor(...)` sets both (constructors are valid `ATTRIBUTE_MEMBER` targets).
- Any struct-field registration helper used by `STRUCT_MEMBER` also updates `lastRegisteredMember` (and clears `lastRegisteredMethod`).

New macros (validation rules are deterministic and happen at type-loading time):
- `ATTRIBUTE_TYPE(TYPE, ...)`
  - Compile-time check: `static_assert(requires { TYPE{ __VA_ARGS__ }; });` (empty `__VA_ARGS__` => `TYPE{}`).
    - Design invariant: attribute types are reflectable, aggregate-like structs (no user-defined constructors).
  - Runtime checks:
    - `GetTypeDescriptor<TYPE>()` must be resolvable at load time (attribute struct descriptors must be registered/loaded before any type uses them).
    - The type descriptor must be a struct (`TypeDescriptorFlags::Struct`).
    - Each argument is boxed into a `Value` and `Value::GetTypeDescriptor()->GetSerializableType()` must be non-null; otherwise raise `Error`.
  - Registration: `RegisterTypeAttribute(...)`.

- `ATTRIBUTE_MEMBER(TYPE, ...)`
  - Requires `lastRegisteredMember != nullptr`; otherwise `CHECK_ERROR` with a clear message (macro misuse).
  - Registration: `RegisterMemberAttribute(lastRegisteredMember, ...)`.

- `ATTRIBUTE_PARAMETER(PARAMETER_NAME, TYPE, ...)`
  - Requires `lastRegisteredMethod != nullptr`; otherwise `CHECK_ERROR` with a clear message (macro misuse).
  - Locate `IParameterInfo*` by name from `lastRegisteredMethod->GetParameter(i)->GetName()`.
    - If not found: `CHECK_ERROR`.
    - If multiple matches (should not happen under valid C++): `CHECK_ERROR` to keep behavior deterministic.
  - Registration: `RegisterMemberAttribute(parameterInfo, ...)`.

Non-target clarification:
- Method groups remain non-attributable (keep `MethodGroupInfoImpl::GetAttribute*` returning empty).

### 4) Metadata (de)serialization changes
Metaonly generation/loading is implemented in `Source\Reflection\DescriptableInterfaces_Metaonly.cpp`:
- `GenerateMetaonlyTypes()` walks all `ITypeDescriptor`s, then methods/properties/events and serializes metadata structs.
- `LoadMetaonlyTypes()` reads the same structs and builds `MetaonlyTypeDescriptor` / `MetaonlyMethodInfo` / etc.

To support attributes:
1. Extend the metadata schema with attribute payloads:
   - Add `AttributeInfoMetadata` (attribute type + serialized values).
   - Add `AttributeValueMetadata` for each argument: `(WString typeName, WString data)` where `data` is produced by `ISerializableType::Serialize`.
     - Use designated initializers for these new aggregate-like metadata structs (repo learning).
2. Add attribute lists to:
   - `TypeDescriptorMetadata` (type-level attributes)
   - `MethodInfoMetadata`, `PropertyInfoMetadata`, `EventInfoMetadata`, `ParameterInfoMetadata` (member-level/parameter-level attributes)
3. In `GenerateMetaonly*` functions, iterate `GetAttributeCount()` for each bag and emit `AttributeInfoMetadata` entries.
4. In metaonly reader objects, override `GetAttributeCountInternal/GetAttributeInternal` to serve attributes from stored metadata (same routing as runtime):
   - For type descriptors: serve from `TypeDescriptorMetadata`.
   - For members/parameters: serve from the member’s metadata.
   - For each attribute value: rebuild `Value` using `context->serializableTypes[typeName]->Deserialize(data, value)`.

Binary compatibility policy:
- Adding these fields is an intentional breaking change to the binary metadata format. This is acceptable because `Metadata_Test` consumes metadata freshly generated by `Metadata_Generate`; old `.bin` files are not expected to remain loadable.
- If the loader encounters an unexpected shape/version, it should fail clearly rather than silently reading incorrect data.

Metaonly resolution / determinism:
- The loading flow should ensure all relevant type/serializable-type registries are available before attribute value materialization, so resolving `typeName` and deserializing `data` is deterministic (consistent with the existing two-pass “create descriptors first, populate later” pattern if applicable).

### 5) Logging updates (`LogTypeManager`)
The `.txt` baseline comparison used by `Metadata_Test` depends on `LogTypeManager` (`Source\Reflection\DescriptableInterfaces_Log.cpp`).
- Add logging for:
  - Type-level attributes
  - Member-level attributes (property/event/method/constructor)
  - Method parameter attributes
- Specify a concrete, deterministic text shape and ordering:
  - Emit type attributes first (registration order), before events/properties/methods.
  - Emit member attributes immediately before the owning member signature, preserving member order.
  - Emit parameter attributes immediately before the owning method signature, preserving parameter order.
  - For each attribute, preserve authored argument order.
- Use a stable representation for arguments based on `ISerializableType::Serialize` (since only serializable primitive values are allowed), e.g.:
  - `    @Attribute:<AttributeTypeName>(<ArgTypeName>:<SerializedData>, ...)`

### 6) Implementation notes (repo learnings)
- Prefer designated initializers for new aggregate-like metadata structs.
- Avoid `collections::Dictionary` copy assignment (use move / in-place construction).
- When accessing `Ptr<T>` values stored in containers, prefer `.Obj()` to obtain raw pointers, consistent with existing code patterns.

## Why `Metadata_Generate` loads predefined types but `Metadata_Test` doesn’t
- `Metadata_Generate\GenerateMetadata.cpp` calls `LoadPredefinedTypes()` and then `GetGlobalTypeManager()->Load()`. This registers all built-in type descriptors from compiled reflection code so `GenerateMetaonlyTypes()` can enumerate them.
- `Metadata_Test\LoadMetadata.cpp` does **not** call `LoadPredefinedTypes()`. Instead, it builds a metaonly type loader from `Reflection{32,64}.bin` via `LoadMetaonlyTypes(...)`, then adds that loader to the global type manager and loads.
- In other words:
  - `Metadata_Generate` source-of-truth = compiled reflection registrations.
  - `Metadata_Test` source-of-truth = binary metadata previously generated.
This separation is intentional for round-trip testing.

## Test plan design (operational)
- Create a shared test pair under `Test\Source` with concrete names, e.g.:
  - `Test\Source\TestReflection_Attribute.h`
  - `Test\Source\TestReflection_Attribute.cpp`
- The shared files define:
  - One reflectable attribute struct (e.g. `MyAttribute`) and its reflection.
  - One test class that exercises all supported targets:
    - `ATTRIBUTE_TYPE` on the type
    - `ATTRIBUTE_MEMBER` on: property / event / method (static + instance) / constructor
    - `ATTRIBUTE_PARAMETER` on at least one method parameter
    - If struct members are supported targets, also cover `STRUCT_MEMBER` + `ATTRIBUTE_MEMBER` after it.
  - One helper function `void TestAttributes()` (declared in `.h`, implemented in `.cpp`) that asserts:
    - Counts, attribute types, and argument values on the type and each member/parameter.
    - Deterministic ordering (registration order for attributes; authored order for values).
    - Use `TEST_ASSERT` only (no `TEST_FILE` / `TEST_CASE` in the shared file).
- Guarding / build modes:
  - Wrap registration and `TestAttributes()` body in `#ifndef VCZH_DEBUG_NO_REFLECTION` so the shared file compiles cleanly when reflection is disabled.
  - Ensure the metaonly path (`VCZH_DEBUG_METAONLY_REFLECTION`) works by relying on `Metadata_Test` loading from generated `.bin`.
- Wire the shared files into both projects (per source-file management rules):
  - Add `TestReflection_Attribute.h/.cpp` to both `Metadata_Generate` and `Metadata_Test` projects in `Test\UnitTest` so the test type exists in both the compiled registration run and the metaonly load run.
- Add test cases:
  - In `Test\Source\TestMetadataGeneration.cpp` (Metadata_Generate): add a `TEST_CASE` that loads predefined types, ensures the test type is registered, then calls `TestAttributes()`. Reset the global type manager afterward following existing patterns.
  - In `Test\Source\TestMetadata.cpp` (Metadata_Test): after loading metadata (see `LoadMetadata.cpp`), call `TestAttributes()`.
    - If `GetTypeDescriptor<TestClass>()` does not resolve in metaonly mode, fall back to `GetTypeDescriptor(L"<TypeName>")` and assert it is non-null.
    - Reset the global type manager afterward following existing patterns.

# AFFECTED PROJECTS
- Always Build the solution in folder REPO-ROOT\Test\UnitTest with Debug|Win32.
- Always Run CLI|UnitTest project Metadata_Generate with Debug|Win32.
- Always Run CLI|UnitTest project Metadata_Test with Debug|Win32.
- Always Build the solution in folder REPO-ROOT\Test\UnitTest with Debug|x64.
- Always Run CLI|UnitTest project Metadata_Generate with Debug|x64.
- Always Run CLI|UnitTest project Metadata_Test with Debug|x64.
- Always Run CLI|UnitTest project VlppReflection with Debug|x64.

# !!!FINISHED!!!

