/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_BOXING_DETAILTYPEINFORETRIVER
#define VCZH_REFLECTION_BOXING_DETAILTYPEINFORETRIVER

#include "TypeInfoRetriver.h"
#include "../Metadata/Metadata.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{ 
/***********************************************************************
Basic Types
***********************************************************************/

			template<typename T>
			struct DetailTypeInfoRetriver<T, TypeFlags::NonGenericType>
			{
				static const ITypeInfo::Decorator						Decorator=ITypeInfo::TypeDescriptor;
				typedef T												Type;
				typedef T												TempValueType;
				typedef T&												ResultReferenceType;
				typedef T												ResultNonReferenceType;

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					return MakePtr<TypeDescriptorTypeInfo>(GetTypeDescriptor<Type>(), hint);
				}
#endif
			};

			template<typename T>
			struct DetailTypeInfoRetriver<const T, TypeFlags::NonGenericType>
			{
				typedef DetailTypeInfoRetriver<T, TypeFlags::NonGenericType>	UpLevelRetriver;

				static const ITypeInfo::Decorator								Decorator=UpLevelRetriver::Decorator;
				typedef typename UpLevelRetriver::Type							Type;
				typedef T														TempValueType;
				typedef const T&												ResultReferenceType;
				typedef const T													ResultNonReferenceType;

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					return TypeInfoRetriver<T>::CreateTypeInfo();
				}
#endif
			};

			template<typename T>
			struct DetailTypeInfoRetriver<volatile T, TypeFlags::NonGenericType>
			{
				typedef DetailTypeInfoRetriver<T, TypeFlags::NonGenericType>	UpLevelRetriver;

				static const ITypeInfo::Decorator								Decorator=UpLevelRetriver::Decorator;
				typedef typename UpLevelRetriver::Type							Type;
				typedef T														TempValueType;
				typedef T&														ResultReferenceType;
				typedef T														ResultNonReferenceType;

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					return TypeInfoRetriver<T>::CreateTypeInfo();
				}
#endif
			};

			template<typename T>
			struct DetailTypeInfoRetriver<T*, TypeFlags::NonGenericType>
			{
				typedef DetailTypeInfoRetriver<T, TypeFlags::NonGenericType>	UpLevelRetriver;

				static const ITypeInfo::Decorator								Decorator=ITypeInfo::RawPtr;
				typedef typename UpLevelRetriver::Type							Type;
				typedef T*														TempValueType;
				typedef T*&														ResultReferenceType;
				typedef T*														ResultNonReferenceType;

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					return MakePtr<RawPtrTypeInfo>(TypeInfoRetriver<T>::CreateTypeInfo());
				}
#endif
			};

			template<typename T>
			struct DetailTypeInfoRetriver<Ptr<T>, TypeFlags::NonGenericType>
			{
				typedef DetailTypeInfoRetriver<T, TypeFlags::NonGenericType>	UpLevelRetriver;

				static const ITypeInfo::Decorator								Decorator=ITypeInfo::SharedPtr;
				typedef typename UpLevelRetriver::Type							Type;
				typedef Ptr<T>													TempValueType;
				typedef Ptr<T>&													ResultReferenceType;
				typedef Ptr<T>													ResultNonReferenceType;

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					return MakePtr<SharedPtrTypeInfo>(TypeInfoRetriver<T>::CreateTypeInfo());
				}
#endif
			};

			template<typename T>
			struct DetailTypeInfoRetriver<Nullable<T>, TypeFlags::NonGenericType>
			{
				typedef DetailTypeInfoRetriver<T, TypeFlags::NonGenericType>	UpLevelRetriver;

				static const ITypeInfo::Decorator								Decorator=ITypeInfo::Nullable;
				typedef typename UpLevelRetriver::Type							Type;
				typedef Nullable<T>												TempValueType;
				typedef Nullable<T>&											ResultReferenceType;
				typedef Nullable<T>												ResultNonReferenceType;

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					return MakePtr<NullableTypeInfo>(TypeInfoRetriver<T>::CreateTypeInfo());
				}
#endif
			};

			template<typename T>
			struct DetailTypeInfoRetriver<T&, TypeFlags::NonGenericType>
			{
				typedef DetailTypeInfoRetriver<T, TypeFlags::NonGenericType>	UpLevelRetriver;

				static const ITypeInfo::Decorator								Decorator=UpLevelRetriver::Decorator;
				typedef typename UpLevelRetriver::Type							Type;
				typedef typename UpLevelRetriver::TempValueType					TempValueType;
				typedef T&														ResultReferenceType;
				typedef T														ResultNonReferenceType;

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					return TypeInfoRetriver<T>::CreateTypeInfo();
				}
#endif
			};

/***********************************************************************
Containers
***********************************************************************/

			template<typename T, typename TCollectionType>
			struct DetailTypeInfoRetriver_Template1
			{
				typedef DetailTypeInfoRetriver<T, TypeFlags::NonGenericType>	UpLevelRetriver;

				static const ITypeInfo::Decorator								Decorator = UpLevelRetriver::Decorator;
				typedef TCollectionType											Type;
				typedef typename UpLevelRetriver::TempValueType					TempValueType;
				typedef typename UpLevelRetriver::ResultReferenceType			ResultReferenceType;
				typedef typename UpLevelRetriver::ResultNonReferenceType		ResultNonReferenceType;

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					typedef typename DetailTypeInfoRetriver<T, TypeFlags::NonGenericType>::Type		ContainerType;
					typedef typename ContainerType::ElementType										ElementType;

					auto arrayType = MakePtr<TypeDescriptorTypeInfo>(GetTypeDescriptor<TCollectionType>(), hint);

					auto genericType = MakePtr<GenericTypeInfo>(arrayType);
					genericType->AddGenericArgument(TypeInfoRetriver<ElementType>::CreateTypeInfo());

					auto type = MakePtr<SharedPtrTypeInfo>(genericType);
					return type;
				}
#endif
			};

			template<typename T, typename TCollectionType>
			struct DetailTypeInfoRetriver_Template2
			{
				typedef DetailTypeInfoRetriver<T, TypeFlags::NonGenericType>	UpLevelRetriver;

				static const ITypeInfo::Decorator								Decorator = UpLevelRetriver::Decorator;
				typedef TCollectionType											Type;
				typedef typename UpLevelRetriver::TempValueType					TempValueType;
				typedef typename UpLevelRetriver::ResultReferenceType			ResultReferenceType;
				typedef typename UpLevelRetriver::ResultNonReferenceType		ResultNonReferenceType;

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					typedef typename DetailTypeInfoRetriver<T, TypeFlags::NonGenericType>::Type		ContainerType;
					typedef typename ContainerType::KeyContainer									KeyContainer;
					typedef typename ContainerType::ValueContainer									ValueContainer;
					typedef typename KeyContainer::ElementType										KeyType;
					typedef typename ValueContainer::ElementType									ValueType;

					auto arrayType = MakePtr<TypeDescriptorTypeInfo>(GetTypeDescriptor<TCollectionType>(), hint);

					auto genericType = MakePtr<GenericTypeInfo>(arrayType);
					genericType->AddGenericArgument(TypeInfoRetriver<KeyType>::CreateTypeInfo());
					genericType->AddGenericArgument(TypeInfoRetriver<ValueType>::CreateTypeInfo());

					auto type = MakePtr<SharedPtrTypeInfo>(genericType);
					return type;
				}
#endif
			};

			template<typename T>
			struct DetailTypeInfoRetriver<T, TypeFlags::EnumerableType> : DetailTypeInfoRetriver_Template1<T, IValueEnumerable>
			{
			};

			template<typename T>
			struct DetailTypeInfoRetriver<T, TypeFlags::ReadonlyListType> : DetailTypeInfoRetriver_Template1<T, IValueReadonlyList>
			{
			};

			template<typename T>
			struct DetailTypeInfoRetriver<T, TypeFlags::ArrayType> : DetailTypeInfoRetriver_Template1<T, IValueArray>
			{
			};

			template<typename T>
			struct DetailTypeInfoRetriver<T, TypeFlags::ListType> : DetailTypeInfoRetriver_Template1<T, IValueList>
			{
			};

			template<typename T>
			struct DetailTypeInfoRetriver<T, TypeFlags::ObservableListType> : DetailTypeInfoRetriver_Template1<T, IValueObservableList>
			{
			};

			template<typename T>
			struct DetailTypeInfoRetriver<T, TypeFlags::ReadonlyDictionaryType> : DetailTypeInfoRetriver_Template2<T, IValueReadonlyDictionary>
			{
			};

			template<typename T>
			struct DetailTypeInfoRetriver<T, TypeFlags::DictionaryType> : DetailTypeInfoRetriver_Template2<T, IValueDictionary>
			{
			};

/***********************************************************************
Functions
***********************************************************************/

#ifndef VCZH_DEBUG_NO_REFLECTION
			namespace internal_helper
			{
				template<typename T>
				struct GenericArgumentAdder
				{
					static void Add(Ptr<GenericTypeInfo> genericType)
					{
					}
				};

				template<typename T0, typename ...TNextArgs>
				struct GenericArgumentAdder<TypeTuple<T0, TNextArgs...>>
				{
					static void Add(Ptr<GenericTypeInfo> genericType)
					{
						genericType->AddGenericArgument(TypeInfoRetriver<T0>::CreateTypeInfo());
						GenericArgumentAdder<TypeTuple<TNextArgs...>>::Add(genericType);
					}
				};
			}
#endif

			template<typename R, typename ...TArgs>
			struct DetailTypeInfoRetriver<Func<R(TArgs...)>, TypeFlags::FunctionType>
			{
				typedef DetailTypeInfoRetriver<const Func<R(TArgs...)>, TypeFlags::NonGenericType>	UpLevelRetriver;
 
				static const ITypeInfo::Decorator								Decorator=UpLevelRetriver::Decorator;
				typedef IValueList												Type;
				typedef typename UpLevelRetriver::TempValueType					TempValueType;
				typedef typename UpLevelRetriver::ResultReferenceType			ResultReferenceType;
				typedef typename UpLevelRetriver::ResultNonReferenceType		ResultNonReferenceType;
 
#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					auto functionType = MakePtr<TypeDescriptorTypeInfo>(GetTypeDescriptor<IValueFunctionProxy>(), hint);
 
					auto genericType = MakePtr<GenericTypeInfo>(functionType);
					genericType->AddGenericArgument(TypeInfoRetriver<R>::CreateTypeInfo());
					internal_helper::GenericArgumentAdder<TypeTuple<TArgs...>>::Add(genericType);

					auto type = MakePtr<SharedPtrTypeInfo>(genericType);
					return type;
				}
#endif
			};

			template<typename R, typename ...TArgs>
			struct DetailTypeInfoRetriver<const Func<R(TArgs...)>, TypeFlags::FunctionType>
				: DetailTypeInfoRetriver<Func<R(TArgs...)>, TypeFlags::FunctionType>
			{
			};
		}
	}
}

#endif
