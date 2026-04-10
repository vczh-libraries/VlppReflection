# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

In TestReflection_Attribute.cpp there is a big test case for attribute macros. Many attribute values are in `vint(xxx)` format. In the macro you can see arguments are tested against the constructor of the specified attribute type. So theoretically you are able to know what the expected argument is.

The goal is to find a way to make `vint(xxx)` being able to write `xxx` instead. But you must also be able to know the type is `vint` (because the attribute type's field type says so). In the same way you should be able to remove the special treatment which hardcoded `WString` for `wchar_t*` arguments.

`vint` is `int64_t` in x64 and `int` in Win32, so if you do it right, `xxx` will not be recognized to `int` but to `vint` (vint is the correct answer, but if you do nothing, xxx will always be int because that's how C++ does it).

By running Metadata_Generation and Metadata_Test, no change should be made to `Reflection(32|64)(\[2\])?.txt`. If they change, it means type inferencing is not right. For example, if `xxx` is inferred to `int`, you will see in Reflection64.txt and Reflection64[2].txt many argument values changed from 64 bits int to 32 bits, e.g.
from `@MyAttribute(system::String:type, system::Int64:1)` to `@MyAttribute(system::String:type, system::Int32:1)`.

# UPDATES

## UPDATE

You have made a semantically correct C++ code to solve the issue which is good. But I'd like to request for a more modern (C++20) way of implementation. Could you try harder to leverage the template variadic argument feature, maybe you also need some type tuple or type list technique, so that you don't need to hard code a zero-to-ten solution? If you can't do that within BoxAttributeFields feel free to do any change, including but not limited to implement it in MakeAttributeInfo, make your own best decision.

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

- No.1 Construct attribute struct then decompose fields for type-aware boxing [DENIED]

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

### DENIED BY USER

The approach is semantically correct and all tests pass, but the user requests a more modern C++20 implementation that eliminates the hardcoded zero-to-ten `if constexpr` chain in `BoxAttributeFields`. The `CountAggregateFields` function and the massive structured-binding decomposition with per-field `if constexpr (ArgCount >= N)` blocks are too verbose and not scalable. The user wants to leverage template variadic arguments and type list techniques instead.

Original test results (all passing):
- `Metadata_Generate` x64: 174/174 passed (includes `TestReflectionAttributes()`)
- `Metadata_Generate` Win32: 174/174 passed
- `Metadata_Test` x64: 174/174 passed (binary round-trip verified)
- `UnitTest`: 53/53 passed
- `git diff Test/Metadata/` shows NO changes to any Reflection*.txt files, confirming:
  - On x64: integer args (e.g., `1`) are correctly inferred as `vint` = `vint64_t`, boxed as `system::Int64`
  - On Win32: integer args are correctly inferred as `vint` = `vint32_t`, boxed as `system::Int32`
  - String args (e.g., `L"type"`) are correctly inferred as `WString`
  - The special hardcoded `wchar_t*` → `WString` and integral → `vint` conversions are successfully eliminated

- No.2 BoxingProxy with template conversion operator during aggregate init [CONFIRMED]

## No.2 BoxingProxy with template conversion operator during aggregate init

Instead of constructing the struct then decomposing it with structured bindings, use a `BoxingProxy<TArg>` wrapper with a `template<typename FieldType> operator FieldType()` conversion operator. During aggregate initialization of `TAttribute`, each proxy is placed in a field position. The compiler deduces `FieldType` from the struct's field type, the conversion operator then:
1. Direct-initializes a `FieldType` from the forwarded argument (performing the same implicit conversion as normal aggregate init)
2. Boxes it with `BoxValue<FieldType>()` using the correct type
3. Returns it to satisfy the aggregate field initialization

This approach needs NO structured bindings, NO field counting, NO hardcoded 0-to-10 cases. The entire mechanism is:
- A single `BoxingProxy` struct template (~15 lines)
- A simplified `MakeAttributeInfo` that does `TAttribute instance{ BoxingProxy<TArgs>{info, args}... };`

The pack expansion `BoxingProxy<TArgs>{info, args}...` naturally handles any number of fields. The boxing happens as a side effect during aggregate initialization, leveraging C++20's template conversion operator deduction in aggregate init context.

### CODE CHANGE

**File: `Source/Reflection/Reflection/Macros.h`**

Replaced the entire `detail::attribute_macro` section (FieldTypeDetector, CountAggregateFields, AddAttributeField, BoxAttributeFields, MakeAttributeInfo — ~140 lines) with just two things (~25 lines):

1. `BoxingProxy<TArg>` — A struct template holding an `AttributeInfoImpl*` and a forwarding reference `TArg&& value`. Has a single `template<typename FieldType> operator FieldType()` conversion operator that:
   - Direct-initializes `FieldType field(std::forward<TArg>(value))` — converting the arg to the struct's field type
   - Boxes it with `BoxValue<std::remove_cvref_t<FieldType>>(field)` — using the correct reflected type
   - Validates the type is serializable
   - Adds the boxed value to the attribute info
   - Returns the field value to complete the aggregate initialization

2. Updated `MakeAttributeInfo<TAttribute>(TArgs&&... args)` — Instead of constructing the struct then decomposing it, uses pack expansion to wrap each arg in a BoxingProxy:
   `TAttribute instance{ BoxingProxy<TArgs>{info.Obj(), std::forward<TArgs>(args)}... };`
   The aggregate initialization drives the conversion: each proxy's `operator FieldType()` is invoked with `FieldType` deduced from the corresponding struct field type.

**File: `Test/Source/TestReflection_Attribute.cpp`** — No additional changes (same as No.1).

### CONFIRMED

All tests passed on both x64 and Win32:
- `Metadata_Generate` x64: 174/174 passed (includes `TestReflectionAttributes()`)
- `Metadata_Generate` Win32: 174/174 passed
- `Metadata_Test` x64: 174/174 passed (binary round-trip verified)
- `UnitTest` x64: 53/53 passed
- `git diff Test/Metadata/` shows NO changes to any Reflection*.txt files
- The code went from ~140 lines (hardcoded 0-to-10 structured binding cases) to ~25 lines (single BoxingProxy + simplified MakeAttributeInfo), with no limit on the number of struct fields