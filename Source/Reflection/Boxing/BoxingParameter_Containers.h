/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_BOXING_BOXINGPARAMETER_CONTAINERS
#define VCZH_REFLECTION_BOXING_BOXINGPARAMETER_CONTAINERS

#include "../Wrappers/ContainerWrappers.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
/***********************************************************************
Containers
***********************************************************************/

			template<typename TValueItf, typename TExpectedItf, template<typename T> class TValueImpl, typename T>
			auto GetValueCollectionFromCollection(T* collection) -> std::enable_if_t<std::is_convertible_v<TValueItf*, TExpectedItf*>, Ptr<TExpectedItf>>
			{
				auto colref = collection->template TryGetCollectionReference<TValueImpl<T*>>();
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

			template<typename T, typename TValueItf>
			Unboxed<T> UnboxCollection(const Ptr<TValueItf>& colref)
			{
				using TCollection = std::remove_const_t<T>;
				if (auto colobj = dynamic_cast<TCollection*>(const_cast<Object*>(colref->GetCollectionObject())))
				{
					return { colobj,false };
				}
				else
				{
					auto collection = new TCollection();
					auto lazyList = GetLazyList<typename TCollection::ElementType>(colref);
					collections::CopyFrom(*collection, lazyList);
					return { collection, true };
				}
			}

			template<typename T, typename TValueItf>
			Unboxed<T> UnboxDictionary(const Ptr<TValueItf>& colref)
			{
				using TCollection = std::remove_const_t<T>;
				if (auto colobj = dynamic_cast<TCollection*>(const_cast<Object*>(colref->GetCollectionObject())))
				{
					return { colobj,false };
				}
				else
				{
					auto collection = new TCollection();
					auto lazyList = GetLazyList<
						typename TCollection::KeyContainer::ElementType,
						typename TCollection::ValueContainer::ElementType
					>(colref);
					collections::CopyFrom(*collection, lazyList);
					return { collection, true };
				}
			}

			template<typename T>
			struct ParameterAccessor<const collections::LazyList<T>, TypeFlags::EnumerableType>
			{
				static Value BoxParameter(const collections::LazyList<T>& object, ITypeDescriptor* typeDescriptor)
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
			};

			template<typename T>
			struct ParameterAccessor<collections::LazyList<T>, TypeFlags::EnumerableType>
			{
				static Value BoxParameter(collections::LazyList<T>& object, ITypeDescriptor* typeDescriptor)
				{
					return ParameterAccessor<const collections::LazyList<T>, TypeFlags::EnumerableType>::BoxParameter(object, typeDescriptor);
				}

				static Unboxed<collections::LazyList<T>> UnboxParameter(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					typedef typename collections::LazyList<T>::ElementType ElementType;
					Ptr<IValueEnumerable> listProxy = UnboxValue<Ptr<IValueEnumerable>>(value, typeDescriptor, valueName);
					return { new collections::LazyList<T>(std::move(GetLazyList<T>(listProxy))), true };
				}
			};

			template<typename T>
			struct ParameterAccessor<T, TypeFlags::ReadonlyListType>
			{
				static Value BoxParameter(T& object, ITypeDescriptor* typeDescriptor)
				{
					return GetValueFromEnumerable<IValueReadonlyList>(object);
				}

				static Unboxed<T> UnboxParameter(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					return UnboxCollection<T>(UnboxValue<Ptr<IValueReadonlyList>>(value, typeDescriptor, valueName));
				}
			};

			template<typename T>
			struct ParameterAccessor<T, TypeFlags::ArrayType>
			{
				static Value BoxParameter(T& object, ITypeDescriptor* typeDescriptor)
				{
					return GetValueFromEnumerable<IValueArray>(object);
				}

				static Unboxed<T> UnboxParameter(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					return UnboxCollection<T>(UnboxValue<Ptr<IValueArray>>(value, typeDescriptor, valueName));
				}
			};

			template<typename T>
			struct ParameterAccessor<T, TypeFlags::ListType>
			{
				static Value BoxParameter(T& object, ITypeDescriptor* typeDescriptor)
				{
					return GetValueFromEnumerable<IValueList>(object);
				}

				static Unboxed<T> UnboxParameter(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					return UnboxCollection<T>(UnboxValue<Ptr<IValueList>>(value, typeDescriptor, valueName));
				}
			};

			template<typename T>
			struct ParameterAccessor<T, TypeFlags::ObservableListType>
			{
				static Value BoxParameter(T& object, ITypeDescriptor* typeDescriptor)
				{
					return GetValueFromEnumerable<IValueObservableList>(object);
				}

				static Unboxed<T> UnboxParameter(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					return UnboxCollection<T>(UnboxValue<Ptr<IValueObservableList>>(value, typeDescriptor, valueName));
				}
			};

			template<typename T>
			struct ParameterAccessor<T, TypeFlags::ReadonlyDictionaryType>
			{
				static Value BoxParameter(T& object, ITypeDescriptor* typeDescriptor)
				{
					return GetValueFromEnumerable<IValueReadonlyDictionary>(object);
				}

				static Unboxed<T> UnboxParameter(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					return UnboxDictionary<T>(UnboxValue<Ptr<IValueReadonlyDictionary>>(value, typeDescriptor, valueName));
				}
			};

			template<typename T>
			struct ParameterAccessor<T, TypeFlags::DictionaryType>
			{
				static Value BoxParameter(T& object, ITypeDescriptor* typeDescriptor)
				{
					return GetValueFromEnumerable<IValueDictionary>(object);
				}

				static Unboxed<T> UnboxParameter(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					return UnboxDictionary<T>(UnboxValue<Ptr<IValueDictionary>>(value, typeDescriptor, valueName));
				}
			};
		}
	}
}

#endif
