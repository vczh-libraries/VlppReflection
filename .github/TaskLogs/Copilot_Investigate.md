# !!!INVESTIGATE!!!
# PROBLEM DESCRIPTION
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

# UPDATES

# TEST [CONFIRMED]

The current implementation cannot represent a partial metadata layer:

- `GenerateMetaonlyTypes(stream::IStream&)` enumerates every descriptor in `GetGlobalTypeManager()`, assigns all of them local `tdIndex` values, and writes the four metadata counts as the first stream fields.
- `LoadMetaonlyTypes` reads those four counts first and constructs every descriptor from records in the same file. There is no dependency header and no way to resolve a descriptor from an already loaded manager.
- `MetaonlyTypeLoader::Load` registers every descriptor in its reader context, so a future foreign prefix would currently be registered a second time.
- The existing metadata generator loads predefined and attribute test types together and emits one self-contained file, so it does not exercise a dependent layer.

The implementation will add and run these tests and verification conditions:

1. `CollectRegisteredTypes` replaces a pre-populated output list with the exact descriptors in a loaded global manager.
2. A self-contained base file serializes an empty dependency-name list and round-trips to the base-only text snapshot.
3. Before generating the attribute layer, the captured base descriptor list is deliberately reversed. The serialized dependency header must nevertheless contain unique registered names in ascending order.
4. The attribute layer header's local type count must equal the number of descriptors added by `CreateTestTypeLoader_Attribute`.
5. Loading the attribute file in a fresh manager must raise the expected `vl::Error` from `LoadMetaonlyTypes`; it must not cause an uncontrolled crash.
6. Loading the base file and then the attribute file must immediately match the base and combined text baselines respectively.
7. The combined round trip must preserve the existing attribute tests, including references from local types to foreign types and non-null, null, and value-type `ITypeDescriptor*` attribute values.
8. Win32 and x64 VlppReflection unit tests, metadata generation, metadata loading, binary-header checks, and all four text comparisons must pass without memory-leak output.
9. The regenerated release must build in VlppParser2 on Win32 and x64.
10. Workflow must independently reject its C++-types layer, then round-trip its base and combined layers on both architectures; all Workflow tests and TypeScript verification must pass.
11. GacUI's independently loadable core and full metadata files must use the new empty-dependency format; the complete GacUI build workflow and `GacUI_Compiler` must pass.
12. Searches of non-generated source/test folders must find no old one-argument `GenerateMetaonlyTypes` call.

# PROPOSALS

- No.1 ADD A SORTED REGISTERED-NAME DEPENDENCY PREFIX AND LOAD METADATA IN LAYERS [CONFIRMED]

## No.1 ADD A SORTED REGISTERED-NAME DEPENDENCY PREFIX AND LOAD METADATA IN LAYERS [CONFIRMED]

Treat every metadata binary as one layer. Its header contains the sorted registered reflection names of descriptors supplied by previously loaded layers; all following method, property, event, attribute, and type-descriptor records remain local to the current layer.

The writer will validate the exclusion snapshot against the loaded global manager, serialize exactly one `List<WString>` dependency header, assign foreign type-descriptor indices first in serialized-name order, and then assign local indices in the existing deterministic name order. Only local types contribute records and the four existing counts.

The reader will resolve the dependency header before reading any counts, retain those descriptors in `MetaonlyReaderContext::tds`, record the first local descriptor index, and append new descriptors after the foreign prefix. Loader registration and type-level attribute reconstruction will start at that local boundary. `ITypeDescriptor` will be resolved from the combined table so descriptor-valued attributes keep their special representation.

`CollectRegisteredTypes` will provide a replace-not-append snapshot API for callers that capture a completed base layer. API comments and the attribute-registration knowledge page will document the loaded-manager lifetime contract, registered-name identity, deterministic ordering, and base-before-dependent loading requirement.

VlppReflection metadata tests will generate separate base and attribute files, reject the attribute layer by itself with `TEST_ERROR`, compare each stage immediately, and inspect the dependency header produced from a deliberately reversed exclusion list. `Project.md` will describe the actual `Test/Metadata` artifacts and both architecture round trips.

After regenerating `VlppReflection/Release`, the generated files will be copied directly to the VlppParser2, Workflow, and GacUI imports. Workflow will split `LoadCppTypes()` metadata into a dependent `ReflectionCppTypes{32,64}.bin` layer with base and combined baselines. GacUI will pass an empty exclusion list to both independent generator calls and regenerate all affected metadata.

### CODE CHANGE

- Added `CollectRegisteredTypes` and changed `GenerateMetaonlyTypes` to accept an exclusion snapshot before the output stream.
- Validated a loaded manager plus non-null, unique, exactly registered exclusions; serialized one sorted `List<WString>` dependency header; assigned foreign type indices before deterministic local indices; emitted records only for local descriptors.
- Added a reader-context local boundary, resolved and retained foreign descriptors before existing counts, registered and reconstructed attributes only for local descriptors, and resolved `ITypeDescriptor` from the combined table.
- Split VlppReflection metadata generation/loading into base and attribute layers, deliberately reversed the caller exclusion order, inspected dependency headers and local counts, added controlled missing-dependency coverage, and compared base and combined snapshots immediately.
- Updated the predefined-type shared test path, `Project.md`, API comments, and the attribute-registration knowledge page.
- Regenerated `VlppReflection/Release` and copied the generated header and source directly to the VlppParser2, Workflow, and GacUI imports.
- Split Workflow metadata into self-contained `Reflection{32,64}.bin` base files and dependent `ReflectionCppTypes{32,64}.bin` files, with immediate base/combined comparisons and independent-layer rejection tests.
- Updated both GacUI metadata generators to pass an empty exclusion list, regenerated the four independently loadable metadata binaries, and refreshed their Tools copies.
- The first Win32 compile exposed that the dynamic missing-name diagnostic must pass a `WString::Buffer()` to `CHECK_ERROR`; the final implementation retains the required name-bearing message in a local `WString`.
- Linking the shared attribute assertions into `Metadata_Test` exposed two pre-existing full-reflection assumptions in that shared file. Its custom `TypeInfo` definitions and registrations now compile only in the generator, while metaonly assertions resolve custom descriptors by registered name and obtain argument types from `IAttributeInfo::GetAttributeValueType`.
- The first VlppParser2 compatibility invocation exposed that its build helper discovers `UnitTest.sln` from the current directory. Invoking it from `Test/UnitTest` then exposed an existing naming mismatch: the solution offers `Debug|x86`, while the helper accepts `Win32` and rejects `x86`. Downstream verification used the helper for `x64` and the same Visual Studio/MSBuild environment directly for the solution's `x86` configuration.
- The first Workflow loader run exposed that resource baselines retain the generator's historical trailing-blank-line formatting, while a direct `LogTypeManager` output does not. The generator reconciles and verifies resource baselines; the immediate loader round trip now compares `[2]` to the corresponding generated snapshot, preserving Workflow's established two-stage contract.
- The GacUI unit-test stage rewrote the unrelated `Application/Dialog_File/OpenAndSelect` snapshot with shifted asynchronous frame IDs and a reordered transient render sequence. The test passed, the snapshot group was clean before the run, and this task changes neither file-dialog behavior nor rendering; those generated side effects were excluded from the task changeset.
- Once the new metadata snapshots were staged, `git diff --check` reported their generator-produced trailing blank line at EOF: four VlppReflection attribute logs and six Workflow C++-type logs/baselines. This is the same `LogTypeManager` output in each baseline and `[2]` round trip, so the generated files remain unchanged; every hand-edited file passes the whitespace check.

### CONFIRMED

The proposal is confirmed by direct format assertions, staged loader tests, complete downstream builds, and artifact review:

- VlppReflection passed Debug Win32 and x64 builds with zero errors. On each architecture, `UnitTest` passed 53/53, `Metadata_Generate` passed 175/175, and `Metadata_Test` passed 174/174 with no memory-leak report.
- The generator verified that `CollectRegisteredTypes` replaces a pre-populated list, deliberately reversed the exclusion snapshot, and then verified an ascending unique dependency header and the exact local descriptor count. The loader's attribute-layer-only call passed through `TEST_ERROR` on both architectures.
- The base and dependent attribute layers loaded in order and exercised foreign base/signature references plus non-null, null, and value-type `ITypeDescriptor*` attribute values. All four VlppReflection base/combined text round trips are byte-identical.
- The regenerated VlppReflection release diff contains the same API and implementation changes as the owning source. Its header and source are byte-identical to every direct import in VlppParser2, Workflow, and GacUI.
- VlppParser2 built successfully in Debug x86 and x64 with zero errors. The x86 solution configuration was invoked directly because the repository helper accepts `Win32` while this solution names the configuration `x86`.
- Workflow passed Debug Win32 and x64 builds; `LibraryTest` passed 15/15, `RuntimeTest` passed 261/261, and `CppTest`, `CppTest_Metaonly`, and `CppTest_Reflection` each passed 229/229 on both hosts. `CompilerTest_GenerateMetadata` passed 2/2, and `CompilerTest_LoadAndCompile` passed 709/709 for both its x86 and x64 scenarios on both hosts, including dependent-layer rejection.
- Workflow TypeScript preparation and compilation passed. The complete `Tools/Tools/Build.ps1 Workflow` pipeline passed twice, and the clean rerun reproduced the same reviewed artifacts. All four Workflow base/combined text round trips are byte-identical.
- `Tools/Tools/Build.ps1 GacUI` completed successfully after rebuilding both Release architectures, running metadata generation/loading and unit tests on each, regenerating the release, and refreshing tool artifacts. The separate repository CLI execution of the x64 `GacUI_Compiler` completed successfully while driving both x86 and x64 resource generation.
- GacUI produced no `*.UI.errors.txt` and no generated C++ changes. Each of its four metadata binaries begins with the expected empty dependency list, and every binary is byte-identical to its copy under `Tools/Tools`.
- Final non-generated source/test searches across all four repositories found no one-argument `GenerateMetaonlyTypes` call. Whitespace checks passed in every hand-edited file and all non-text generated artifacts; only the ten new generator-produced text snapshots/baselines report their intentional trailing blank line. Tools is clean, and every remaining change in the other repositories is an expected source, generated release/import, metadata, or baseline artifact.

The registered-name dependency prefix therefore preserves stable type-descriptor indices across layers while preventing foreign metadata from being serialized or registered twice. Resolving and retaining dependencies before local records makes the base-before-dependent contract fail early and keeps all foreign references valid for the dependent loader's lifetime.
