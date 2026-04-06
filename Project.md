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
- Build the solution with Debug|x64.
- Run `Metadata_Generate` with Debug|x64.
- Run `Metadata_Test` with Debug|x64.

It generates binary metadata files containing type informations from reflection code.

`Metadata_Generate` produces `Test/UnitTest/Metadata/Reflection{32,64}.bin` and `Test/UnitTest/Metadata/Reflection{32,64}.txt`.
`Metadata_Test` loads the `.bin` file, regenerates a `[2].txt` file, and compares it against the `.txt` baseline to verify binary serialization round-trip correctness.

If the schema of reflected types has changed, the baseline `.txt` file may no longer match.
This is expected. To update the baseline, re-run `Metadata_Generate` for both Win32 and x64,
which overwrites `Test/UnitTest/Metadata/Reflection{32,64}.txt` with the updated type information.
Then re-run `Metadata_Test` to confirm the round-trip is correct.
