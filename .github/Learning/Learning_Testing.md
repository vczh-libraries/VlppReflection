# !!!LEARNING!!!

# Orders

- Verify reflection metadata format changes on both Win32 and x64 [1]

# Refinements

## Verify reflection metadata format changes on both Win32 and x64

When attribute serialization or binary metadata layout changes, build Debug Win32 and Debug x64, run `Metadata_Generate` for both architectures, then run `Metadata_Test` so the regenerated `[2]` dumps match the base `Reflection32.txt` and `Reflection64.txt` outputs. Also run the reflection unit tests that exercise runtime attribute values.
