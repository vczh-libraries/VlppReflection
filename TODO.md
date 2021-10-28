# TODO

## 2.0

- Add DisposedFlag to collections and all reflectable classes and interfaces.
- When a reflectable object is being created against a collection:
  - Regardless of what interface type is created, a most detailed writable interface is created and stored into the collection.
    - When seeing `IEnumerable<T>`, need to figure out the underlying type.
  - If the pointer is not `nullptr`, use that object, throws when the object from the pointer is not detailed enough.
- Dump and reload type metadata with some types not included.
  - Version number in binary file.
  - List dependency type, check if all of them are available before loading this file.
    - Or implement delay loading like GacUI resources.
- Move platform-dependent code to separated files as what `Vlpp` does.

## Optional
