# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

`AttributeValueMetadata` and `AttributeInfoMetadata` using `WString` as type name is not a high performance choice.
`MetaonlyTypeInfo` uses `vint typeDescriptor;`, align with this.

In the reflection to binary metadata serialization and deserialization you can find the code around this. In the high level picture, all ITypeDescriptor instances are collected into a list so that all ITypeDescriptor can be converted to an index bidirectionally. I believe AttributeValueMetadata and AttributeInfoMetadata is in the same case. Attribute type is just ITypeDescriptor, it can be directly processed. Attribute value must be serializable primitive type, so ITypeDescriptor is good enough to determine its type instead of using the type AST (ITypeInfo).

Currently both ITypeDescriptor for attribute type and attribute value type are serialized to its full name, I want you to do the vint version.

To test the code, you should run Metadata_Generation followed by Metadata_Test in debug with both x64 and Win32. This is a performance improvement so you won't see any test failure right now, but they should help verify your code change.

After running the test with your patch, Reflection32.bin and Reflection64.bin should change, but all text files in the same folder (text format metadata dump) should not change, or if they change there should be just changing in type orders. But anyway keep an eye on text files should help you find any issue if test fails.

ReflectionWithTestTypes(32|64).txt are produced by UnitTest project. After your patch being verified by Metadata_Generation and Metadata_Test, you can then run UnitTest in debug with both x64 and Win32 to do a final check.

# UPDATES

# TEST [CONFIRMED]

The existing Metadata_Generate and Metadata_Test infrastructure serves as the test for this change. There are no new test cases to write since this is a performance/serialization format improvement.

**Test Procedure:**
1. Build solution with Debug|Win32, run Metadata_Generate with Debug|Win32.
2. Build solution with Debug|x64, run Metadata_Generate with Debug|x64.
3. Run Metadata_Test with Debug|x64 (verifies binary round-trip by comparing regenerated `.txt` against baseline).
4. Run Metadata_Test with Debug|Win32.
5. Run UnitTest with Debug|Win32 and Debug|x64 as final check.

**Success Criteria:**
- All Metadata_Generate runs succeed (produces `Reflection32.bin`, `Reflection64.bin`, `Reflection32.txt`, `Reflection64.txt`).
- All Metadata_Test runs succeed (`.txt` round-trip matches baseline).
- `Reflection32.bin` and `Reflection64.bin` change compared to before patch (different serialization format).
- `Reflection32.txt` and `Reflection64.txt` should not change, or only have type order differences.
- UnitTest passes for both Win32 and x64; `ReflectionWithTestTypes32.txt` and `ReflectionWithTestTypes64.txt` should be unchanged or have only type order changes.
- No test regression.

# PROPOSALS

- No.1 Replace WString with vint typeDescriptor indices in AttributeValueMetadata and AttributeInfoMetadata [CONFIRMED]

## No.1 Replace WString with vint typeDescriptor indices in AttributeValueMetadata and AttributeInfoMetadata

Change `AttributeValueMetadata::typeName` from `WString` to `vint typeDescriptor` and `AttributeInfoMetadata::attributeTypeName` from `WString` to `vint attributeType`. Update serialization macros accordingly. Update `GenerateMetaonlyAttributes` to accept `MetaonlyWriterContext&` and use `tdIndex` to convert `ITypeDescriptor*` to `vint`. Update `LoadMetaonlyAttributes` to look up type descriptors via `context->tds[index]` directly, removing the need for the `typeDescriptors` dictionary parameter.

Detailed changes:
1. In `AttributeValueMetadata`: change `WString typeName;` to `vint typeDescriptor = -1;`, update serialization.
2. In `AttributeInfoMetadata`: change `WString attributeTypeName;` to `vint attributeType = -1;`, update serialization.
3. In `GenerateMetaonlyAttributes`: add `MetaonlyWriterContext&` parameter, use `context.tdIndex[...]` to convert `ITypeDescriptor*` to index.
4. In `LoadMetaonlyAttributes`: replace name lookups with direct index lookups via `context->tds[...]`, remove the `typeDescriptors` parameter.
5. Update all call sites of both functions accordingly.

### CODE CHANGE

All changes in `Source/Reflection/DescriptableInterfaces_Metaonly.cpp`:

1. **Struct `AttributeValueMetadata`**: Changed `WString typeName;` to `vint typeDescriptor = -1;`.
2. **Struct `AttributeInfoMetadata`**: Changed `WString attributeTypeName;` to `vint attributeType = -1;`.
3. **Serialization macros**: Updated `SERIALIZE(typeName)` to `SERIALIZE(typeDescriptor)` and `SERIALIZE(attributeTypeName)` to `SERIALIZE(attributeType)`.
4. **`GenerateMetaonlyAttributes`**: Added `MetaonlyWriterContext& context` as first parameter. Uses `context.tdIndex[info->GetAttributeType()]` and `context.tdIndex[valueType]` instead of `->GetTypeName()`.
5. **All `GenerateMetaonlyAttributes` call sites** (5 total): Updated to pass `*writer.context.Obj()` as first argument.
6. **`LoadMetaonlyAttributes`**: Removed `const Dictionary<WString, ITypeDescriptor*>& typeDescriptors` parameter. Uses `context->tds[attributeMetadata->attributeType]` and `context->tds[valueMetadata->typeDescriptor]` directly instead of name-based dictionary lookups. Gets `serializableType` from the resolved `reflectedValueType->GetSerializableType()` instead of `context->serializableTypes`.
7. **All `LoadMetaonlyAttributes` call sites** (5 total): Removed `typeDescriptors` argument.
8. **Removed unused `typeDescriptors` dictionary** construction in `LoadMetaonlyTypes`.

### CONFIRMED

All tests pass:
- Metadata_Generate with Debug|Win32: 174/174 test cases passed.
- Metadata_Generate with Debug|x64: 174/174 test cases passed.
- Metadata_Test with Debug|x64: 174/174 test cases passed (binary round-trip verified).
- Metadata_Test with Debug|Win32: 174/174 test cases passed.
- UnitTest with Debug|Win32: 53/53 test cases passed.
- UnitTest with Debug|x64: 53/53 test cases passed.

Only `Reflection32.bin` and `Reflection64.bin` changed (49656 -> 49084 bytes, ~1.2% smaller). All `.txt` files (Reflection32.txt, Reflection64.txt, ReflectionWithTestTypes32.txt, ReflectionWithTestTypes64.txt) are unchanged, confirming the semantic equivalence of the serialization.

