The goal of this task is to add a new feature to VlppReflection's metaonly binary type information dumping.
Currently `GenerateMetaonlyTypes` dump every type from `GetGlobalTypeManager` in a binary file.
I would like to add a feature, so it supports dumping partial files by:
- Add `CollectRegisteredTypes(List<ITypeDescriptor*>&)` function, use `GetGGlobalTypeManager` and its `GetTypeDescriptorCount` and `GetTypeDescriptor` to collect all types to a list.
- Add `List<ITypeDescriptor*>& excludedTypes` as the first argument to `GenerateMetaonlyTypes`:
  - The binary format is going to change, this is a breaking change.
  - Note that there is a `tds` map in this function, it eventually generates `tdIndex`, that's how an `ITypeDescriptor*` is serialized as a number in the binary format.
  - You will need to add these data at the beginning of the binary file:
    - An number for `excludeTypes.Count()`.
    - A list of full name translated from `excludedTypes`.
    - You can build a `Sorted<WString>`, `CopyFrom` to a `List<WString>` and write it directly, this type is supported.
  - When building `tdIndex`, `excludedTypes` should be assigned first in the order of type names.
  - And in the later part, content excluded types should not appear in the binary file.
  - So that the binary can declare "these types are foreign types, they are supposed to be ready when LoadMetaonlyTypes is called".
- The `excludedTypes` can be empty.
- Update `LoadMetaonlyTypes` and check the list, if any type does not exist yet, call `CHECK_ERROR` or `CHECK_FAIL` according to how you write the code.
  - Declare a `List<WString>` and read the whole list from the binary file, this type is supported.
  - Remember that `List<WString>` now represents number 0..Count()-1 of `ITypeDescriptor*` which is already registered instead of later loading from the binary file.
- `excludedTypes` is highly possibly be referenced in types in the binary file, so the above way to keep the `tdIndex` stable and easy to reconstruct is important.

To verify:
- Update `VlppReflection::Metadata_Generate`'s `GenerateMetadata.cpp` so that:
  - Delete `LoadPredefinedTypesForTestCase` and its content should be inlined into the test case below.
  - After calling `LoadPredefinedTypes`, call `GenerateMetaonlyTypes` to write Reflection(32|64).bin.
  - Now call `CollectRegisteredTypes`, `CreateTestTypeLoader_Attribute`, `GenerateMetaonlyTypes` again to write ReflectionAttribute(32|64).bin
  - To make dumping easire, `ReflectionAttribute(32|64)([2])?.txt` would include content of both `Reflection(32|64).bin` and `ReflectionAttribute(32|64).bin`.
- Update `VlppReflection::Metadata_Test`'s `LoadMetadata.cpp`:
  - Load `ReflectionAttribute(32|64).bin` and make sure it crashes.
  - Load `Reflection(32|64).bin` followed by `ReflectionAttribute(32|64).bin`.
    - After loading each file, the baseline should be checked right away, this would be easier to do, and that's why `ReflectionAttribute(32|64)([2])?.txt` would include content of both `Reflection(32|64).bin` and `ReflectionAttribute(32|64).bin`.
- Release `VlppReflection` to `VlppParser2` and make sure it builds, no test for `VlppParser2` is required.
- Release `VlppReflection` to `Workflow`.
  - `CompilterTest_GeneratedMetadata` and `CompilerTest_LoadAndCompiler` needs to be fixed just like `Metadata_Generate` and `Metadata_Test`.
  - `LoadCppTypes()` should be another binary file `ReflectionCppTypes(32|64).bin` depending on `Reflection(32|64).bin`.
  - Run every test files of `Workflow` to make sure it works.
- Release `VlppReflection` to `GacUI`.
  - No source change is required except that calling to `GenerateMetaonlyTypes` needs an empty list argument, but since the metadata file format are changed, you need to run `Tools/Tools/Build.ps1 GacUI` to update everything, followed by `GacUI_Compiler` to make sure the compiler works with the new metadata binary files.
