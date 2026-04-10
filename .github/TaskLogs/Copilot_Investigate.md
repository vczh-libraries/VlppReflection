# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

In TestReflection_Attribute.cpp there is a big test case for attribute macros. Many attribute values are in `vint(xxx)` format. In the macro you can see arguments are tested against the constructor of the specified attribute type. So theoretically you are able to know what the expected argument is.

The goal is to find a way to make `vint(xxx)` being able to write `xxx` instead. But you must also be able to know the type is `vint` (because the attribute type's field type says so). In the same way you should be able to remove the special treatment which hardcoded `WString` for `wchar_t*` arguments.

`vint` is `int64_t` in x64 and `int` in Win32, so if you do it right, `xxx` will not be recognized to `int` but to `vint` (vint is the correct answer, but if you do nothing, xxx will always be int because that's how C++ does it).

By running Metadata_Generation and Metadata_Test, no change should be made to `Reflection(32|64)(\[2\])?.txt`. If they change, it means type inferencing is not right. For example, if `xxx` is inferred to `int`, you will see in Reflection64.txt and Reflection64[2].txt many argument values changed from 64 bits int to 32 bits, e.g.
from `@MyAttribute(system::String:type, system::Int64:1)` to `@MyAttribute(system::String:type, system::Int32:1)`.

# UPDATES

# TEST [CONFIRMED]

The existing test infrastructure covers this change:

1. `TestReflectionAttributes()` in `TestReflection_Attribute.cpp` verifies attribute values at runtime via `AssertMyAttribute()` which checks that integer values match expectations using `ReadAttributeInteger()`. This function already handles both `vint32_t` and `vint64_t` so it supports cross-platform.
2. `Metadata_Generate` generates binary metadata and text dump (`Reflection32.txt`, `Reflection64.txt`).
3. `Metadata_Test` loads binary metadata, regenerates `[2].txt` files, compares against `.txt` baseline for round-trip correctness.

**Test Procedure:**
1. Change `vint(xxx)` to `xxx` in the ATTRIBUTE_* macros in `TestReflection_Attribute.cpp`.
2. Modify `BoxAttributeArgument` / `MakeAttributeInfo` in `Macros.h` to infer boxing types from struct field types instead of hardcoded rules.
3. Build solution with Debug|Win32, run `Metadata_Generate` with Debug|Win32.
4. Build solution with Debug|x64, run `Metadata_Generate` with Debug|x64.
5. Run `Metadata_Test` with Debug|x64.
6. Run `UnitTest` with Debug|x64.

**Success Criteria:**
- All builds and test runs succeed.
- `Reflection32.txt`, `Reflection64.txt`, `Reflection32[2].txt`, `Reflection64[2].txt` remain unchanged (especially integer types must stay as `system::Int64` on x64 and `system::Int32` on Win32, not change to `system::Int32` on x64).
- `TestReflectionAttributes()` passes on both platforms.

# PROPOSALS

- No.1 Construct attribute struct then decompose fields for type-aware boxing [CONFIRMED]

## No.1 Construct attribute struct then decompose fields for type-aware boxing

Instead of boxing each argument based on its C++ deduced type (with hardcoded special cases for `wchar_t*` → `WString` and all integrals → `vint`), construct the attribute struct via aggregate initialization first (`TAttribute instance{ args... }`), then decompose the struct into its fields using C++20 structured bindings and box each field using its actual struct field type.

This approach:
1. Uses C++20 `requires` clauses to count aggregate fields at compile time (`CountAggregateFields<T>()`)
2. Uses structured bindings to decompose the aggregate into a tuple of references (`StructToTuple`)
3. Boxes only the first `sizeof...(TArgs)` fields, each with its correct field type
4. Removes all hardcoded type conversions (`wchar_t*` → `WString`, integral → `vint`)

Since aggregate initialization implicitly converts `1` (int) to `vint` (the field type), the boxing will correctly use `vint` (= `int64_t` on x64, `int32_t` on Win32), preserving the existing Reflection*.txt output.

### CODE CHANGE

**File: `Source/Reflection/Reflection/Macros.h`**

Replaced the old `BoxAttributeArgument`, `AddAttributeArgument`, and `MakeAttributeInfo` functions in `detail::attribute_macro` namespace with:

1. `FieldTypeDetector` — a struct with `template<typename T> constexpr operator T() const noexcept;` used to detect aggregate field count via `requires` clauses.
2. `CountAggregateFields<T>()` — a `consteval` function that determines the number of fields in an aggregate type T using the `FieldTypeDetector` pattern, supporting up to 10 fields.
3. `AddAttributeField(info, field)` — boxes a field using `BoxValue<std::remove_cvref_t<T>>(field)`, preserving the actual field type.
4. `BoxAttributeFields<ArgCount>(info, instance)` — decomposes the struct using structured bindings based on total field count (`CountAggregateFields<TAttribute>()`), then boxes the first `ArgCount` fields.
5. `MakeAttributeInfo<TAttribute>(args...)` — constructs `TAttribute instance{ args... }` first (so C++ aggregate initialization converts args to the correct field types), then calls `BoxAttributeFields<sizeof...(TArgs)>()`.

The key insight: by constructing the struct first, C++ aggregate initialization implicitly converts `1` (int) to `vint` (the field type), and `L"text"` (const wchar_t[]) to `WString`. Then when we decompose and box, each field already has its correct type.

**File: `Test/Source/TestReflection_Attribute.cpp`**

Changed all `vint(xxx)` to just `xxx` in ATTRIBUTE_TYPE, ATTRIBUTE_MEMBER, and ATTRIBUTE_PARAMETER macro calls:
- `vint(91)` → `91`, `vint(1)` → `1`, `vint(2)` → `2`, ..., `vint(9)` → `9`

### CONFIRMED

All tests passed on both x64 and Win32:
- `Metadata_Generate` x64: 174/174 passed (includes `TestReflectionAttributes()`)
- `Metadata_Generate` Win32: 174/174 passed
- `Metadata_Test` x64: 174/174 passed (binary round-trip verified)
- `UnitTest`: 53/53 passed
- `git diff Test/Metadata/` shows NO changes to any Reflection*.txt files, confirming:
  - On x64: integer args (e.g., `1`) are correctly inferred as `vint` = `vint64_t`, boxed as `system::Int64`
  - On Win32: integer args are correctly inferred as `vint` = `vint32_t`, boxed as `system::Int32`
  - String args (e.g., `L"type"`) are correctly inferred as `WString`
  - The special hardcoded `wchar_t*` → `WString` and integral → `vint` conversions are successfully eliminated