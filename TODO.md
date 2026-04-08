# TODO

## Attribute Metadata

- `ATTRIBUTE_TYPE`, `ATTRIBUTE_MEMBER`, `ATTRIBUTE_PARAMETER` is allowed inside a type or right after a member to add attributes.
  - Method macros should check parameter count, if it is less than actual parameter count, all the rest should be omittable.
  - `ATTRIBUTE_PARAMETER` should also specify the parameter name and it will be verified.
- An attribute must be a reflectable struct, type and field values will be offered by above macros.
- All reflection metadata object offers methods to retrieve attributes including there types and arguments.
- `ITypeAttribute` offers attribute type and arguments instead of the whole struct.
- Offer metadata serialization.
- Add test only attributes and attributed metadata for testing.

## 2.0

- Dump and reload type metadata with some types not included.
  - Version number in binary file.
  - List dependency type, check if all of them are available before loading this file.
    - Or implement delay loading like GacUI resources.
- `ObservableListBase` should `CHECK_ERROR` on index out of bound, instead of just returning false.

## Optional
