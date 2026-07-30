# General Instruction

## Solution to Work On

You are working on the solution `REPO-ROOT/Test/UnitTest/UnitTest.sln`,
therefore `SOLUTION-ROOT` is `REPO-ROOT/Test/UnitTest`.

## Projects for Verification

The `REPO-ROOT/Test/UnitTest/VlppReflection/VlppReflection.vcxproj` is the unit test project.
When any *.h or *.cpp file is changed, unit test is required to run.

When any test case fails, you must fix the issue immediately, even those errors are unrelated to the issue you are working on.

### Metadata_Generate and Metadata_Test

These two projects need to run if any reflection code is touched.

To execute these projects, you should:
- Build the solution with Debug|Win32.
- Run `Metadata_Generate` with Debug|Win32.
- Run `Metadata_Test` with Debug|Win32.
- Build the solution with Debug|x64.
- Run `Metadata_Generate` with Debug|x64.
- Run `Metadata_Test` with Debug|x64.

It generates binary metadata files containing type informations from reflection code.

`Metadata_Generate` produces these artifacts in `Test/Metadata`:
- `Reflection{32,64}.bin` is the self-contained predefined-type layer.
- `Reflection{32,64}.txt` is the predefined-type snapshot.
- `ReflectionAttribute{32,64}.bin` is the attribute-test layer, which depends on the corresponding predefined-type layer.
- `ReflectionAttribute{32,64}.txt` is the combined predefined-and-attribute snapshot.

`Metadata_Test` verifies that the attribute layer fails to load by itself, loads the two layers in order, regenerates each `[2].txt` file, and immediately compares each result against its `.txt` baseline.

If the schema of reflected types has changed, the baseline `.txt` file may no longer match.
This is expected. To update the baseline, re-run `Metadata_Generate` for both Win32 and x64,
which overwrites the corresponding `Test/Metadata/*.txt` files with the updated type information.
Then re-run `Metadata_Test` to confirm the round-trip is correct.

## Linux/macOS Specific

`REPO-ROOT/Test/Linux` stores linux configurations for:
- `Metadata_Generate`: `Metadata_Generate.vcxproj`.
- `Metadata_Test`: `Metadata_Test.vcxproj`.
- `UnitTest`: `UnitTest.vcxproj`.

You need to build, test and debug in that specific folder, otherwise the unit test will not function properly.
On Linux, only configuration "debug x64" is available, no need to build or run projects with other configurations.
Unlike Windows, building have to be done in each folder separately.
