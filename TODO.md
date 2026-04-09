# TODO

## Attribute Metadata (Verification)

- `ATTRIBUTE_*` when a attribute field type is `vint`, argument `10` should work instead of having to write `vint(10)`.
- `AttributeValueMetadata` and `AttributeInfoMetadata` using `WString` as type name is not a high performance choice.
  - `MetaonlyTypeInfo` uses `vint typeDescriptor;`, align with this.
- Update comment/document/KB.

## 2.0

- Dump and reload type metadata with some types not included.
  - Version number in binary file.
  - List dependency type, check if all of them are available before loading this file.
    - Or implement delay loading like GacUI resources.
- `ObservableListBase` should `CHECK_ERROR` on index out of bound, instead of just returning false.
- Method macros should check parameter count, if it is less than actual parameter count, all the rest should be omittable.

## Optional
