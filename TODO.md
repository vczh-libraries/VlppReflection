# TODO

## 2.0

- Is it possible to check if the offered argument names match registered argument count?
- Dump and reload type metadata with some types not included.
  - Version number in binary file.
  - List dependency type, check if all of them are available before loading this file.
    - Or implement delay loading like GacUI resources.
- `ObservableListBase` should `CHECK_ERROR` on index out of bound, instead of just returning false.

## Optional
