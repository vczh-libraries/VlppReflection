# TODO

## 2.0

- Add DisposedFlag to collections and all reflectable classes and interfaces.
- Reflectable interfaces like `IReflectableReadableCollection<T>`, inherits from `IReadableCollection<T>` and `IReadableCollection<Value>`.
- Dump and reload type metadata with some types not included.
  - Version number in binary file.
  - List dependency type, check if all of them are available before loading this file.
    - Or implement delay loading like GacUI resources.
- Move platform-dependent code to separated files as what `Vlpp` does.

## Optional
