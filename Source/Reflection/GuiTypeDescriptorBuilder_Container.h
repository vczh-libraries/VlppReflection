/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/
 
#ifndef VCZH_REFLECTION_GUITYPEDESCRIPTORBUILDER_CONTAINER
#define VCZH_REFLECTION_GUITYPEDESCRIPTORBUILDER_CONTAINER
 
#include "GuiTypeDescriptorWrappers.h"
 
namespace vl
{
	namespace collections
	{
/***********************************************************************
DetailTypeInfoRetriver<TContainer>
***********************************************************************/

		template<typename T>
		void ObservableList<T>::NotifyUpdateInternal(vint start, vint count, vint newCount)
		{
			if (auto colref = this->TryGetCollectionReference<reflection::description::IValueObservableList>())
			{
				colref->ItemChanged(start, count, newCount);
			}
		}
	}

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

			template<typename TValueItf, typename TExpectedItf, template<typename T> class TValueImpl, typename T>
			auto GetValueCollectionFromCollection(T* collection) -> std::enable_if_t<std::is_convertible_v<TValueItf*, TExpectedItf*>, Ptr<TExpectedItf>>
			{
				auto colref = collection->TryGetCollectionReference<TValueImpl<T*>>();
				if (colref) return colref;
				colref = MakePtr<TValueImpl<T*>>(collection);
				collection->SetCollectionReference(colref);
				return colref;
			}

			template<typename TValueItf, typename TExpectedItf, template<typename T> class TValueImpl, typename T>
			auto GetValueCollectionFromCollection(T* collection) -> std::enable_if_t<!std::is_convertible_v<TValueItf*, TExpectedItf*>, Ptr<TExpectedItf>>
			{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				throw ArgumentTypeMismtatchException(
					WString::Unmanaged(L"collection"),
					Description<TExpectedItf>::GetAssociatedTypeDescriptor(),
					Description<TValueItf>::GetAssociatedTypeDescriptor()
					);
#else
				CHECK_FAIL(L"vl::reflection::description::GetValueCollectionFromCollection()#Argument type mismatch.");
#endif
			}

			template<typename TValueItf, typename T>
			Ptr<TValueItf> ChooseValueCollectionFromNonDictionaryEnumerable(const collections::IEnumerable<T>* enumerable)
			{
				auto writable = const_cast<Object*>(enumerable->GetCollectionObject());
				if (auto xs = dynamic_cast<collections::ObservableList<T>*>(writable))
				{
					return GetValueCollectionFromCollection<IValueObservableList, TValueItf, ValueObservableListWrapper>(xs);
				}
				else if (auto xs = dynamic_cast<collections::ObservableListBase<T>*>(writable))
				{
					return GetValueCollectionFromCollection<IValueList, TValueItf, ValueListWrapper>(xs);
				}
				else if (auto xs = dynamic_cast<collections::List<T>*>(writable))
				{
					return GetValueCollectionFromCollection<IValueList, TValueItf, ValueListWrapper>(xs);
				}
				else if (auto xs = dynamic_cast<collections::Array<T>*>(writable))
				{
					return GetValueCollectionFromCollection<IValueArray, TValueItf, ValueArrayWrapper>(xs);
				}
				else if (auto xs = dynamic_cast<collections::SortedList<T>*>(writable))
				{
					return GetValueCollectionFromCollection<IValueReadonlyList, TValueItf, ValueReadonlyListWrapper>(xs);
				}
				else
				{
					return GetValueCollectionFromCollection<IValueEnumerable, TValueItf, ValueEnumerableWrapper>(xs);
				}
			}

			template<typename TValueItf, typename T>
			Ptr<TValueItf> ChooseValueCollectionFromEnumerable(const collections::IEnumerable<T>* enumerable)
			{
				return ChooseValueCollectionFromNonDictionaryEnumerable<TValueItf>(enumerable);
			}

			template<typename TValueItf, typename K, typename V>
			Ptr<TValueItf> ChooseValueCollectionFromEnumerable(const collections::IEnumerable<collections::Pair<K, V>>* enumerable)
			{
				auto writable = const_cast<Object*>(enumerable->GetCollectionObject());
				if (auto xs = dynamic_cast<collections::Dictionary<K, V>*>(writable))
				{
					return GetValueCollectionFromCollection<IValueDictionary, TValueItf, ValueDictionaryWrapper>(xs);
				}
				else
				{
					return ChooseValueCollectionFromNonDictionaryEnumerable<TValueItf>(enumerable);
				}
			}

			template<typename TValueItf, typename T>
			Value GetValueFromEnumerable(const collections::IEnumerable<T>& enumerable)
			{
				auto result = ChooseValueCollectionFromEnumerable<TValueItf>(&enumerable);
				ITypeDescriptor* td = nullptr;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				td = Description<TValueItf>::GetAssociatedTypeDescriptor();
#endif
				return BoxValue(result, td);
			}

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
					return BoxValue(result, td);
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
					return GetValueFromEnumerable<IValueReadonlyList>(object);
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
					return GetValueFromEnumerable<IValueArray>(object);
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
					return GetValueFromEnumerable<IValueList>(object);
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
					return GetValueFromEnumerable<IValueObservableList>(object);
				}
			};

			template<typename T>
			struct ParameterAccessor<T, TypeFlags::ReadonlyDictionaryType>
			{
				static Value BoxParameter(T& object, ITypeDescriptor* typeDescriptor)
				{
					return GetValueFromEnumerable<IValueReadonlyDictionary>(object);
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
					return GetValueFromEnumerable<IValueDictionary>(object);
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