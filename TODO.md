# TODO

## 2.0

- Add DisposedFlag to collections and all reflectable classes and interfaces.
- Redesign `BoxParameter` and `UnboxParameter`.
  - `UnboxParameter` could return a reference holder, it is not necessary to create a new object if a referenced native object could be retrived from the wrapper.
- Dump and reload type metadata with some types not included.
  - Version number in binary file.
  - List dependency type, check if all of them are available before loading this file.
    - Or implement delay loading like GacUI resources.
- Move platform-dependent code to separated files as what `Vlpp` does.

## Optional
