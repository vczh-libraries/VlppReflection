# TODO

## 2.0

- Dump and reload type metadata with some types not included.
  - Version number in binary file.
  - List dependency type, check if all of them are available before loading this file.
    - Or implement delay loading like GacUI resources.
  - Need a way to easily specify which types are not included in the current generated metadata:
    - Maybe a function to read type list from previous generated metadata, and use the same list to exclude these types from the current generated metadata.
- `ObservableListBase` should `CHECK_ERROR` on index out of bound, instead of just returning false.
- Method macros should check parameter count, if it is less than actual parameter count, all the rest should be omittable.

## Optional
