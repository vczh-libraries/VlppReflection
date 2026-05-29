# !!!LEARNING!!!

# Orders

- `AttributeInfoImpl::AddValue` owns attribute value validation [1]
- Encode `ITypeDescriptor*` attributes separately from serialized values [1]
- Qualify `description::GetTypeDescriptor<T>()` in reflection macros and tests [1]

# Refinements

## `AttributeInfoImpl::AddValue` owns attribute value validation

Keep attribute value validation in `AttributeInfoImpl::AddValue` so every producer path, including `BoxingProxy` and metaonly metadata loading, follows the same rule. Attribute values are normally serializable struct values; `ITypeDescriptor*` is the explicit exception and must be represented by a raw pointer or null, not a shared pointer or arbitrary boxed object.

## Encode `ITypeDescriptor*` attributes separately from serialized values

Binary attribute metadata should distinguish descriptor references from serialized argument payloads. Store an `ITypeDescriptor*` attribute as a referenced type-descriptor index with an empty serialized data string; store ordinary serializable attribute values with the descriptor-reference slot empty and the serialized data populated.

## Qualify `description::GetTypeDescriptor<T>()` in reflection macros and tests

When reflection test macros or attribute declarations appear in a context that can also see `DescriptableObject::GetTypeDescriptor()`, qualify the free template as `description::GetTypeDescriptor<T>()`. This avoids member-name shadowing when passing descriptor values into attributes.
