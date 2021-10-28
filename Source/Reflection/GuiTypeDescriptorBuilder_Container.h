/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/
 
#ifndef VCZH_REFLECTION_GUITYPEDESCRIPTORBUILDER_CONTAINER
#define VCZH_REFLECTION_GUITYPEDESCRIPTORBUILDER_CONTAINER
 
#include "GuiTypeDescriptorBuilder.h"
 
namespace vl
{
	namespace reflection
	{
		namespace description
		{

/***********************************************************************
DetailTypeInfoRetriver<TContainer>
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
ParameterAccessor<TContainer>
***********************************************************************/

			template<typename T>
			struct ParameterAccessor<collections::LazyList<T>, TypeFlags::EnumerableType>
			{
				static Value BoxParameter(collections::LazyList<T>& object, ITypeDescriptor* typeDescriptor)
				{
					Ptr<IValueEnumerable> result = IValueEnumerable::Create(
						collections::From(object)
							.Select([](const T& item)
							{
								return BoxValue<T>(item);
							})
						);

					ITypeDescriptor* td = nullptr;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
					td = Description<IValueEnumerable>::GetAssociatedTypeDescriptor();
#endif
					return BoxValue<Ptr<IValueEnumerable>>(result, td);
				}

				static void UnboxParameter(const Value& value, collections::LazyList<T>& result, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					typedef typename collections::LazyList<T>::ElementType ElementType;
					Ptr<IValueEnumerable> listProxy = UnboxValue<Ptr<IValueEnumerable>>(value, typeDescriptor, valueName);
					result = GetLazyList<T>(listProxy);
				}
			};

			template<typename T>
			struct ParameterAccessor<T, TypeFlags::ReadonlyListType>
			{
				static Value BoxParameter(T& object, ITypeDescriptor* typeDescriptor)
				{
					Ptr<IValueReadonlyList> result = new ValueReadonlyListWrapper<T*>(&object);

					ITypeDescriptor* td = nullptr;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
					td = Description<IValueReadonlyList>::GetAssociatedTypeDescriptor();
#endif
					return BoxValue<Ptr<IValueReadonlyList>>(result, td);
				}

				static void UnboxParameter(const Value& value, T& result, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					typedef typename T::ElementType ElementType;
					Ptr<IValueReadonlyList> listProxy = UnboxValue<Ptr<IValueReadonlyList>>(value, typeDescriptor, valueName);
					collections::LazyList<ElementType> lazyList = GetLazyList<ElementType>(listProxy);
					collections::CopyFrom(result, lazyList);
				}
			};

			template<typename T>
			struct ParameterAccessor<T, TypeFlags::ArrayType>
			{
				static Value BoxParameter(T& object, ITypeDescriptor* typeDescriptor)
				{
					Ptr<IValueArray> result = new ValueArrayWrapper<T*>(&object);

					ITypeDescriptor* td = nullptr;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
					td = Description<IValueArray>::GetAssociatedTypeDescriptor();
#endif
					return BoxValue<Ptr<IValueArray>>(result, td);
				}

				static void UnboxParameter(const Value& value, T& result, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					typedef typename T::ElementType ElementType;
					Ptr<IValueArray> arrayProxy = UnboxValue<Ptr<IValueArray>>(value, typeDescriptor, valueName);
					collections::LazyList<ElementType> lazyList = GetLazyList<ElementType>(arrayProxy);
					collections::CopyFrom(result, lazyList);
				}
			};

			template<typename T>
			struct ParameterAccessor<T, TypeFlags::ListType>
			{
				static Value BoxParameter(T& object, ITypeDescriptor* typeDescriptor)
				{
					Ptr<IValueList> result = new ValueListWrapper<T*>(&object);

					ITypeDescriptor* td = nullptr;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
					td = Description<IValueList>::GetAssociatedTypeDescriptor();
#endif
					return BoxValue<Ptr<IValueList>>(result, td);
				}

				static void UnboxParameter(const Value& value, T& result, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					typedef typename T::ElementType ElementType;
					Ptr<IValueList> listProxy = UnboxValue<Ptr<IValueList>>(value, typeDescriptor, valueName);
					collections::LazyList<ElementType> lazyList = GetLazyList<ElementType>(listProxy);
					collections::CopyFrom(result, lazyList);
				}
			};

			template<typename T>
			struct ParameterAccessor<collections::ObservableList<T>, TypeFlags::ObservableListType>
			{
				static Value BoxParameter(collections::ObservableList<T>& object, ITypeDescriptor* typeDescriptor)
				{
					ITypeDescriptor* td = nullptr;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
					td = Description<IValueObservableList>::GetAssociatedTypeDescriptor();
#endif
					return BoxValue<Ptr<IValueObservableList>>(object.GetWrapper(), td);
				}
			};

			template<typename T>
			struct ParameterAccessor<T, TypeFlags::ReadonlyDictionaryType>
			{
				static Value BoxParameter(T& object, ITypeDescriptor* typeDescriptor)
				{
					Ptr<IValueReadonlyDictionary> result = new ValueReadonlyDictionaryWrapper<T*>(&object);

					ITypeDescriptor* td = nullptr;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
					td = Description<IValueReadonlyDictionary>::GetAssociatedTypeDescriptor();
#endif
					return BoxValue<Ptr<IValueReadonlyDictionary>>(result, td);
				}

				static void UnboxParameter(const Value& value, T& result, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					typedef typename T::KeyContainer					KeyContainer;
					typedef typename T::ValueContainer					ValueContainer;
					typedef typename KeyContainer::ElementType			KeyType;
					typedef typename ValueContainer::ElementType		ValueType;

					Ptr<IValueReadonlyDictionary> dictionaryProxy = UnboxValue<Ptr<IValueReadonlyDictionary>>(value, typeDescriptor, valueName);
					collections::LazyList<collections::Pair<KeyType, ValueType>> lazyList = GetLazyList<KeyType, ValueType>(dictionaryProxy);
					collections::CopyFrom(result, lazyList);
				}
			};

			template<typename T>
			struct ParameterAccessor<T, TypeFlags::DictionaryType>
			{
				static Value BoxParameter(T& object, ITypeDescriptor* typeDescriptor)
				{
					Ptr<IValueDictionary> result = new ValueDictionaryWrapper<T*>(&object);

					ITypeDescriptor* td = nullptr;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
					td = Description<IValueDictionary>::GetAssociatedTypeDescriptor();
#endif
					return BoxValue<Ptr<IValueDictionary>>(result, td);
				}

				static void UnboxParameter(const Value& value, T& result, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					typedef typename T::KeyContainer					KeyContainer;
					typedef typename T::ValueContainer					ValueContainer;
					typedef typename KeyContainer::ElementType			KeyType;
					typedef typename ValueContainer::ElementType		ValueType;

					Ptr<IValueDictionary> dictionaryProxy = UnboxValue<Ptr<IValueDictionary>>(value, typeDescriptor, valueName);
					collections::LazyList<collections::Pair<KeyType, ValueType>> lazyList = GetLazyList<KeyType, ValueType>(dictionaryProxy);
					collections::CopyFrom(result, lazyList);
				}
			};
		}
	}
}

#endif