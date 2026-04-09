# TODO

## Attribute Metadata (Verification)

- Is attribute metadata serialized with high performance format?
- Is attribute metadata serialized correctly in text format?
- Method macros should check parameter count, if it is less than actual parameter count, all the rest should be omittable.
- Check test cases.
- Update comment/document/KB.

## 2.0

- Dump and reload type metadata with some types not included.
  - Version number in binary file.
  - List dependency type, check if all of them are available before loading this file.
    - Or implement delay loading like GacUI resources.
- `ObservableListBase` should `CHECK_ERROR` on index out of bound, instead of just returning false.

## Optional
