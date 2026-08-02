# !!!LEARNING!!!

# Orders

- Verify reflection metadata format changes on both Win32 and x64 [2]

# Refinements

## Verify reflection metadata format changes on both Win32 and x64

When attribute serialization or binary metadata layout changes, build Debug Win32 and Debug x64, run `Metadata_Generate` for both architectures, then run `Metadata_Test`. Compare each base and dependent-layer `[2]` dump immediately with its generated snapshot, and cover missing dependencies with `TEST_ERROR` instead of an uncontrolled process crash. Also run the reflection unit tests that exercise runtime attribute values.
