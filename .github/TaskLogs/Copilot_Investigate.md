# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Add `ITypeDescriptor*` as an allowed attribute value type (in addition to serializable struct types). This is an exception to the serializable-only rule.

Changes needed:
1. Update `AttributeInfoImpl::AddValue` validation: if `valueType` points to `ITypeDescriptor` and value has a raw pointer but not a shared pointer or it is null, this is an `ITypeDescriptor*` type. Otherwise they should be serializable struct types. Move existing checks from `BoxingProxy` into `AddValue`.
2. Update `BoxingProxy` in `Macros.h` to handle `ITypeDescriptor*` field types.
3. Update text format logging in `DescriptableInterfaces_Log.cpp` to handle `ITypeDescriptor*` values.
4. Update binary metadata format in `DescriptableInterfaces_Metaonly.cpp`: change `AttributeValueMetadata` from `(int, string)` to `(int, int, string)` representing `(ITypeDescriptor*, ITypeDescriptor*, serialized-argument-value)`. If the value type is `ITypeDescriptor*`, the first int value counts and the data string is empty; vice versa.
5. Add `ITypeDescriptor*` attribute values to the test cases containing `MyAttribute` and `EmptyAttribute`.
6. Expect binary metadata to change. Run all test projects in debug with both Win32 and x64.

# UPDATES

## UPDATE

You are going to scan through all attribute related code (not just what you have done in this session), GetTypeDescriptor<T>() or GetTypeDescriptor(typename) ensure only one instance through the process for one type, so you can just compare two ITypeDescriptor* too see if they are the same type. Comparing type names are not necessary. Avoid any `TypeInfo<T>::content.typeName` as your best effort, only leave one that can't be changed, and list all of these

# TEST

The existing test infrastructure covers this change:

1. `TestReflectionAttributes()` in `TestReflection_Attribute.cpp` verifies attribute values at runtime.
2. `Metadata_Generate` generates binary metadata and text dump (`Reflection32.txt`, `Reflection64.txt`).
3. `Metadata_Test` loads binary metadata, regenerates `[2].txt` files, compares against `.txt` baseline for round-trip correctness.

**Test Procedure:**
1. Add `ITypeDescriptor*` field to `MyAttribute` struct.
2. Update test attribute usages to pass `ITypeDescriptor*` values.
3. Update `BoxingProxy` to handle pointer field types.
4. Move validation into `AttributeInfoImpl::AddValue`.
5. Update text logging for `ITypeDescriptor*` values.
6. Update binary metadata format with `typeDescriptorValue` field.
7. Build solution with Debug|Win32, run `Metadata_Generate` with Debug|Win32.
8. Build solution with Debug|x64, run `Metadata_Generate` with Debug|x64.
9. Run `Metadata_Test` with Debug|x64 and Debug|Win32.
10. Run `VlppReflection` with Debug|x64.

**Success Criteria:**
- All builds and test runs succeed.
- `Reflection32.txt` / `Reflection64.txt` and their `[2]` variants will change (to include `ITypeDescriptor*` values), and the `[2]` files must match the base files.
- `TestReflectionAttributes()` passes on both platforms.

# PROPOSALS

- No.1 Add ITypeDescriptor* support via BoxingProxy pointer path and AddValue validation [DENIED]

## No.1 Add ITypeDescriptor* support via BoxingProxy pointer path and AddValue validation

### CODE CHANGE

**Source/Reflection/Metadata/Metadata.cpp** - `AddValue` validation
- Uses `TypeInfo<ITypeDescriptor>::content.typeName` (static constant, works before type manager loads) to identify ITypeDescriptor* values
- ITypeDescriptor* values must be `Value::RawPtr` or `Value::Null`; all other types must have a serializable type

**Source/Reflection/Reflection/Macros.h** - `BoxingProxy`
- Added `if constexpr (std::is_pointer_v<std::remove_cvref_t<FieldType>>)` branch
- For pointer types: gets `valueType` via `description::GetTypeDescriptor<PointedType>()`, boxes with `BoxValue`
- For non-pointer types: boxes first, gets `valueType` from `boxed.GetTypeDescriptor()`

**Source/Reflection/DescriptableInterfaces_Log.cpp** - `LogTypeManager_FormatAttribute`
- For ITypeDescriptor* values: outputs `typeName:referencedTypeName` or `typeName:null`
- Uses `TypeInfo<ITypeDescriptor>::content.typeName` for type comparison

**Source/Reflection/DescriptableInterfaces_Metaonly.cpp** - Binary metadata
- `AttributeValueMetadata`: added `vint typeDescriptorValue = -1` field
- `GenerateMetaonlyAttributes`: ITypeDescriptor* sets `typeDescriptorValue` to td index, `data` empty; serializable sets `typeDescriptorValue = -1`, `data` = serialized
- `LoadMetaonlyAttributes`: Uses `WString::Unmanaged(TypeInfo<ITypeDescriptor>::content.typeName)` for comparison; creates `Value::From(dynamic_cast<DescriptableObject*>(td))` or `Value()` for null

**Test/Source/TestReflection_Attribute.cpp / .h** - Test cases
- `MyAttribute` struct: added `ITypeDescriptor* type = nullptr` field
- Attribute usages pass `description::GetTypeDescriptor<vint>()`, `description::GetTypeDescriptor<WString>()`, `description::GetTypeDescriptor<bool>()`, or `nullptr`
- `AssertMyAttribute` validates 3rd argument type and value

**Key insights:**
- Must use `TypeInfo<ITypeDescriptor>::content.typeName` instead of `GetTypeDescriptor<ITypeDescriptor>()->GetTypeName()` because `LoadMetaonlyTypes` runs before the type manager is loaded
- Must qualify `description::GetTypeDescriptor<T>()` in test macros because `DescriptableObject::GetTypeDescriptor()` member shadows the free template
- Metaonly type descriptors are different objects than runtime ones, so type name comparison (not pointer equality) is required

### DENIED BY USER
All tests passed, but the user pointed out that `GetTypeDescriptor<T>()` or `GetTypeDescriptor(typename)` ensures only one instance per type throughout the process, so `ITypeDescriptor*` pointer comparison is sufficient. Using `TypeInfo<T>::content.typeName` for type name string comparison is unnecessary overhead. The code should use direct pointer equality (`== GetTypeDescriptor<ITypeDescriptor>()`) wherever possible, and only keep `TypeInfo<T>::content.typeName` in places where it truly cannot be replaced (e.g., when the type manager is not loaded yet).

- No.2 Minimize TypeInfo usage: pointer comparison where possible, serializable check in AddValue [CONFIRMED]

## No.2 Minimize TypeInfo usage: pointer comparison where possible, serializable check in AddValue

Analysis of 4 locations using `TypeInfo<ITypeDescriptor>::content.typeName`:

1. **`DescriptableInterfaces_Log.cpp:33`** (`LogTypeManager_FormatAttribute`) — Type manager IS loaded when logging. Replace with `valueType == GetTypeDescriptor<ITypeDescriptor>()`.
2. **`Metadata.cpp:254`** (`AddValue`) — Called from both runtime (BoxingProxy) and metaonly (LoadMetaonlyAttributes) paths. In metaonly path, valueType is a MetaonlyTypeDescriptor which is a different object from `GetTypeDescriptor<ITypeDescriptor>()`. Solution: flip logic to check `valueType->GetSerializableType() != nullptr` first (serializable path), else validate ITypeDescriptor* (RawPtr/Null). No type name needed.
3. **`DescriptableInterfaces_Metaonly.cpp:1035`** (`GenerateMetaonlyAttributes`) — Type manager IS loaded when generating. Replace with `valueType == GetTypeDescriptor<ITypeDescriptor>()`.
4. **`DescriptableInterfaces_Metaonly.cpp:1090`** (`LoadMetaonlyAttributes`) — Type manager NOT loaded. `reflectedValueType` is a MetaonlyTypeDescriptor from `context->tds`. Cannot call `GetTypeDescriptor<ITypeDescriptor>()`. Solution: find the ITypeDescriptor td from `context->tds` by type name ONCE before the loop, then use pointer comparison inside the loop. This requires one `TypeInfo<ITypeDescriptor>::content.typeName` usage that cannot be eliminated.

### CODE CHANGE

(To be filled after implementation)

