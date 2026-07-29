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

## DETAILS

The following clarifications are authoritative where the original wording contains a typo or shorthand.

### Core API and binary layout

- The API names and types are `GetGlobalTypeManager` and `collections::SortedList<WString>`; there is no `GetGGlobalTypeManager` or `Sorted<WString>`.
- `CollectRegisteredTypes` replaces the output list instead of appending to it. The type manager must already be loaded, and registration or reset must not happen while the snapshot is being used.
- `GenerateMetaonlyTypes` validates that every excluded descriptor is non-null, unique, and is the exact descriptor currently registered under its name. The excluded list can be empty.
- A foreign type is identified by `ITypeDescriptor::GetTypeName()`, which is the registered reflection name used by `ITypeManager`; do not use `ITypeDescriptor::ICpp::GetFullName()`.
- Sort the foreign registered names independently of both the caller's list order and `ITypeManager::GetTypeDescriptor(vint)` order. The latter has no ordering guarantee.
- Write exactly one `List<WString>` containing the sorted foreign names at the beginning of the stream. `List<T>` serialization already writes its count, so do not write a second standalone count.
- Assign `tdIndex` for foreign descriptors first, in exactly the serialized-name order, and then assign local descriptors in the existing deterministic type-name order.
- The serialized `tdCount` is the number of local type-descriptor records only. Valid type-descriptor indices span `foreignNames.Count() + tdCount`.
- Enumerate methods, properties, events, attributes, and type-descriptor records only from local types. Foreign names occur in the dependency header, but no foreign metadata records occur in the layer.
- Method, property, and event indices remain local to the layer and do not receive a foreign prefix.

### Loading a layer

- Read and resolve the foreign-name list before reading the four existing metadata counts. Resolve each name against the already loaded global type manager and fail early with an `ERROR_MESSAGE_PREFIX` message that includes the missing name.
- Append resolved foreign descriptors to `MetaonlyReaderContext::tds` in serialized-name order, record the first local type-descriptor index, and append newly deserialized descriptors after that boundary.
- An empty dependency list keeps the binary self-contained and must not require or initialize pre-existing global reflection state.
- `MetaonlyTypeLoader::Load` registers only descriptors at or after the first local index. It must not attempt to register the foreign prefix again.
- Attribute reconstruction for type descriptors also starts at the first local index. Processing the foreign prefix there would cast native or previously loaded descriptors to `MetaonlyTypeDescriptor` and would attempt to register their attributes in the wrong reader context.
- Resolve `MetaonlyReaderContext::itdTd` from the combined foreign-and-local descriptor table. In the attribute layer, `ITypeDescriptor` itself is foreign; failing to recognize it would send `ITypeDescriptor*` attribute values through the ordinary serializable-value path.
- Keep the resolved foreign descriptors alive in the reader context so all local base types, signatures, generic arguments, and `ITypeDescriptor*` attribute values continue to reference the already registered descriptor objects.

### VlppReflection metadata projects

- Metadata artifacts are stored in `Test/Metadata`, not `Test/UnitTest/Metadata`. Update `Project.md` to describe the base and attribute-layer artifacts and the Win32 round-trip run added below.
- Deleting `LoadPredefinedTypesForTestCase` from `Metadata_Generate/GenerateMetadata.cpp` alone would leave an unresolved call from `Test/Source/TestPredefinedTypes.cpp`.
- Keep `TestPredefinedTypes.cpp` in `Metadata_Generate` and preserve its full-reflection coverage: in its non-metaonly path, directly call `LoadPredefinedTypes()` and load the manager; in its metaonly path, keep the external hook supplied by `Metadata_Test/LoadMetadata.cpp`. This allows the generator helper itself to be deleted and inlined as requested without changing project source lists.

Use this order in `Metadata_Generate`:

1. Call `LoadPredefinedTypes()`, obtain the global type manager, and call `ITypeManager::Load()`.
2. Generate `Reflection{32,64}.bin` with an empty excluded list, then write the base-only `Reflection{32,64}.txt`.
3. Call `CollectRegisteredTypes` after the predefined types have loaded and before adding the attribute loader.
4. Add `CreateTestTypeLoader_Attribute()` with `ITypeManager::AddTypeLoader`. Since the manager is already loaded, the new loader is applied immediately; do not call `Load()` again.
5. Run `TestReflectionAttributes()`.
6. Generate `ReflectionAttribute{32,64}.bin` with the captured base descriptors excluded.
7. Write `ReflectionAttribute{32,64}.txt` by calling `LogTypeManager` once after both layers are registered. It is one snapshot of the combined manager, not a concatenation of two log files.
8. Reset the global type manager.

The new committed attribute-layer artifacts are:

- `Test/Metadata/ReflectionAttribute32.bin`
- `Test/Metadata/ReflectionAttribute32.txt`
- `Test/Metadata/ReflectionAttribute32[2].txt`
- `Test/Metadata/ReflectionAttribute64.bin`
- `Test/Metadata/ReflectionAttribute64.txt`
- `Test/Metadata/ReflectionAttribute64[2].txt`

In `Metadata_Test`:

- Keep the metadata-test `LoadPredefinedTypesForTestCase` for the shared metaonly predefined-type tests, and make it register only the base metadata loader.
- Preserve separate `Ptr<ITypeLoader>` values for the base and attribute layers. Do not construct the attribute loader during global-storage initialization because its dependencies are not registered then.
- In a fresh manager, open `ReflectionAttribute{32,64}.bin` and wrap `LoadMetaonlyTypes` itself in `TEST_ERROR`. The expected `vl::Error` is a passing assertion; do not cause an uncontrolled process crash. Reset the manager and reopen the stream before the positive path.
- For the positive path, load and register `Reflection{32,64}.bin`, write `Reflection{32,64}[2].txt`, and compare it immediately with `Reflection{32,64}.txt`.
- Without resetting, open and load `ReflectionAttribute{32,64}.bin`, add its returned loader to the already loaded manager, write `ReflectionAttribute{32,64}[2].txt`, and compare it immediately with `ReflectionAttribute{32,64}.txt`.
- Reset the global type manager after each architecture's scenario.

### Workflow split

- The exact project names are `CompilerTest_GenerateMetadata` and `CompilerTest_LoadAndCompile`.
- `LoadCppTypes()` registers a type loader; it does not load a binary file. `ReflectionCppTypes{32,64}.bin` is the new dependent binary containing the types contributed by that loader.
- In `CompilerTest_GenerateMetadata`, register and load the predefined, Parser2, XML, JSON, and Workflow-library types first. Generate the self-contained `Reflection{32,64}.bin`, log the base snapshot, and collect the registered descriptors before calling `LoadCppTypes()`.
- Because the manager is already loaded, `LoadCppTypes()` registers its types immediately. Generate `ReflectionCppTypes{32,64}.bin` with the captured base descriptors excluded; do not call `ITypeManager::Load()` again.
- Keep `Reflection{32,64}.txt` as the base-only snapshot. Add `ReflectionCppTypes{32,64}.txt` as one `LogTypeManager` snapshot after both layers are registered, with corresponding `ReflectionCppTypes{32,64}[2].txt` round-trip output and baselines under `Test/Resources/Baseline`.
- In `CompilerTest_LoadAndCompile`, the base loader must be added and activated before even calling `LoadMetaonlyTypes` for `ReflectionCppTypes{32,64}.bin`, because dependency validation happens while reading the dependent file.
- Verify that loading `ReflectionCppTypes{32,64}.bin` by itself raises the expected error for both target architectures. Then reset and perform base load, base comparison, dependent load, and combined comparison in that order.
- Remove the unused duplicate `LoadTypes64()` or update it consistently; do not leave a one-file loading path behind.
- Review generator-produced baseline changes before rerunning `CompilerTest_LoadAndCompile`, because the current generator overwrites a mismatched baseline.

### Release and downstream propagation

- Regenerate `VlppReflection/Release/VlppReflection.h` and `VlppReflection.cpp` from the owning source before updating any downstream import.
- Copy those generated files directly to each of `VlppParser2/Import`, `Workflow/Import`, and `GacUI/Import`. These imports are direct and are not propagated transitively through another dependency's release. Never hand-edit an import copy.
- `VlppParser2` has no non-import caller of either metadata function; after refreshing its import, building both architectures is sufficient as requested.
- In GacUI, update both calls in `Test/GacUISrc/Metadata_Generate/Main.cpp`: the `ReflectionCore{32,64}.bin` call and the `Reflection{32,64}.bin` call. Both use an empty exclusion list and remain independently loadable; the full file must not depend on the core file.
- `Tools/Tools/Build.ps1 GacUI` updates and tests the four GacUI metadata binaries and the copies under `Tools/Tools`, but it does not execute `GacUI_Compiler`; keep the explicit compiler run afterward.
- Update the existing API comments in `Source/Reflection/DescriptableInterfaces.h` and the existing metaonly description in `.github/KnowledgeBase/KB_VlppReflection_AttributeRegistration.md` to document registered-name dependencies and layered loading.

## VERIFICATION

### VlppReflection

1. Build `Test/UnitTest/UnitTest.sln` in `Debug|Win32` with the repository build script.
2. Run `UnitTest`, `Metadata_Generate`, and `Metadata_Test` in `Debug|Win32`.
3. Build the solution in `Debug|x64`.
4. Run `UnitTest`, `Metadata_Generate`, and `Metadata_Test` in `Debug|x64`.
5. Read `Build.log` or `Execute.log` immediately after each script invocation because the next invocation overwrites it. Require successful summaries and no appended memory-leak report.
6. Confirm the missing-dependency cases pass through `TEST_ERROR` for both architectures.
7. Confirm all four text round trips compare equal:
   - `Reflection32.txt` and `Reflection32[2].txt`
   - `ReflectionAttribute32.txt` and `ReflectionAttribute32[2].txt`
   - `Reflection64.txt` and `Reflection64[2].txt`
   - `ReflectionAttribute64.txt` and `ReflectionAttribute64[2].txt`
8. Confirm each base snapshot contains only predefined types and each attribute snapshot contains the combined base and attribute types.
9. Inspect each attribute binary's dependency header and local count: names equal the captured base descriptors, are unique and ascending, and the local count equals the number of newly registered attribute types. This must also be tested with an intentionally reordered exclusion input so correctness does not depend on the type manager's current ordering.
10. Confirm the layered round trip exercises base-type/signature references and the existing non-null, null, and value-type `ITypeDescriptor*` attribute values.
11. Regenerate `VlppReflection/Release` and verify its generated diff before copying it downstream.

### Downstream repositories

1. Refresh the direct `VlppReflection` import in `VlppParser2`; build its unit-test solution in `Debug|Win32` and `Debug|x64`. No VlppParser2 test execution is required.
2. Refresh the direct import in Workflow and implement the two-file split. Run Workflow's required sequence for both architectures, including `CompilerTest_GenerateMetadata`, `CompilerTest_LoadAndCompile`, `LibraryTest`, `RuntimeTest`, and all `CppTest` variants, then run the TypeScript verification.
3. Run the complete `Tools/Tools/Build.ps1 Workflow` workflow, review all new `ReflectionCppTypes*` binaries, generated logs, and baselines, and require a clean rerun.
4. Refresh GacUI's direct import and update both generator calls with empty exclusions.
5. Run `Tools/Tools/Build.ps1 GacUI`, which covers metadata generation/loading and the GacUI unit tests and refreshes the metadata copies used by the tools.
6. Run `GacUI_Compiler` through the repository CLI execution script. Its x64 executable drives both x86 and x64 resource compilation; require a successful exit.
7. Inspect GacUI `git status` for generated C++ changes and untracked `*.UI.errors.txt`. Rebuild and test generated C++ if it changed, and treat any error file as a failure.
8. Search all non-generated source and test folders in the four repositories and confirm no old one-argument `GenerateMetaonlyTypes` call remains.
9. Inspect `git status` and diffs in `VlppReflection`, `VlppParser2`, `Workflow`, `GacUI`, and `Tools`; ensure only expected source, generated release/import, metadata, baseline, and generated-code changes are present.

## REVIEW COMMENTS
