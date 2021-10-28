/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/
 
#ifndef VCZH_REFLECTION_GUITYPEDESCRIPTORBUILDER_WRAPPERS
#define VCZH_REFLECTION_GUITYPEDESCRIPTORBUILDER_WRAPPERS
 
#include "GuiTypeDescriptorPredefined.h"
 
namespace vl
{
	namespace reflection
	{
		namespace description
		{

/***********************************************************************
Enumerable Wrappers
***********************************************************************/

			template<typename T>
			class TypedEnumerator : public Object, public collections::IEnumerator<T>
			{
			private:
				Ptr<IValueEnumerable>		enumerable;
				Ptr<IValueEnumerator>		enumerator;
				vint						index;
				T							value;

			public:
				TypedEnumerator(Ptr<IValueEnumerable> _enumerable, vint _index, const T& _value)
					:enumerable(_enumerable)
					,index(_index)
					,value(_value)
				{
					enumerator=enumerable->CreateEnumerator();
					vint current=-1;
					while(current++<index)
					{
						enumerator->Next();
					}
				}

				TypedEnumerator(Ptr<IValueEnumerable> _enumerable)
					:enumerable(_enumerable)
					,index(-1)
				{
					Reset();
				}

				collections::IEnumerator<T>* Clone()const override
				{
					return new TypedEnumerator<T>(enumerable, index, value);
				}

				const T& Current()const override
				{
					return value;
				}

				vint Index()const override
				{
					return index;
				}

				bool Next() override
				{
					if(enumerator->Next())
					{
						index++;
						value=UnboxValue<T>(enumerator->GetCurrent());
						return true;
					}
					else
					{
						return false;
					}
				}

				void Reset() override
				{
					index=-1;
					enumerator=enumerable->CreateEnumerator();
				}
			};

			/// <summary>Convert a reflectable container to a lazy list to the known element type.</summary>
			/// <returns>The created lazy list.</returns>
			/// <typeparam name="T">The expected element type.</typeparam>
			/// <param name="value">The reflectable container.</param>
			template<typename T>
			collections::LazyList<T> GetLazyList(Ptr<IValueEnumerable> value)
			{
				return collections::LazyList<T>(new TypedEnumerator<T>(value));
			}

			/// <summary>Convert a reflectable container to a lazy list to the known element type.</summary>
			/// <returns>The created lazy list.</returns>
			/// <typeparam name="T">The expected element type.</typeparam>
			/// <param name="value">The reflectable container.</param>
			template<typename T>
			collections::LazyList<T> GetLazyList(Ptr<IValueReadonlyList> value)
			{
				return collections::Range<vint>(0, value->GetCount())
					.Select([value](vint i)
					{
						return UnboxValue<T>(value->Get(i));
					});
			}

			/// <summary>Convert a reflectable container to a lazy list to the known element type.</summary>
			/// <returns>The created lazy list.</returns>
			/// <typeparam name="T">The expected element type.</typeparam>
			/// <param name="value">The reflectable container.</param>
			template<typename T>
			collections::LazyList<T> GetLazyList(Ptr<IValueArray> value)
			{
				return GetLazyList<T>(Ptr<IValueReadonlyList>(value));
			}

			/// <summary>Convert a reflectable container to a lazy list to the known element type.</summary>
			/// <returns>The created lazy list.</returns>
			/// <typeparam name="T">The expected element type.</typeparam>
			/// <param name="value">The reflectable container.</param>
			template<typename T>
			collections::LazyList<T> GetLazyList(Ptr<IValueList> value)
			{
				return GetLazyList<T>(Ptr<IValueReadonlyList>(value));
			}

			/// <summary>Convert a reflectable container to a lazy list to the known element type.</summary>
			/// <returns>The created lazy list.</returns>
			/// <typeparam name="T">The expected element type.</typeparam>
			/// <param name="value">The reflectable container.</param>
			template<typename T>
			collections::LazyList<T> GetLazyList(Ptr<IValueObservableList> value)
			{
				return GetLazyList<T>(Ptr<IValueReadonlyList>(value));
			}

			/// <summary>Convert a reflectable dictionary to a lazy list to the known element type.</summary>
			/// <returns>The created lazy list.</returns>
			/// <typeparam name="K">The expected key type.</typeparam>
			/// <typeparam name="V">The expected value type.</typeparam>
			/// <param name="value">The reflectable dictionary.</param>
			template<typename K, typename V>
			collections::LazyList<collections::Pair<K, V>> GetLazyList(Ptr<IValueReadonlyDictionary> value)
			{
				return collections::Range<vint>(0, value->GetCount())
					.Select([value](vint i)
					{
						return collections::Pair<K, V>(UnboxValue<K>(value->GetKeys()->Get(i)), UnboxValue<V>(value->GetValues()->Get(i)));
					});
			}

			/// <summary>Convert a reflectable dictionary to a lazy list to the known element type.</summary>
			/// <returns>The created lazy list.</returns>
			/// <typeparam name="K">The expected key type.</typeparam>
			/// <typeparam name="V">The expected value type.</typeparam>
			/// <param name="value">The reflectable dictionary.</param>
			template<typename K, typename V>
			collections::LazyList<collections::Pair<K, V>> GetLazyList(Ptr<IValueDictionary> value)
			{
				return GetLazyList<K, V>(Ptr<IValueReadonlyDictionary>(value));
			}

/***********************************************************************
Collection Wrappers
***********************************************************************/

			namespace trait_helper
			{
				template<typename T>
				struct RemovePtr
				{
					typedef T					Type;
				};
				
				template<typename T>
				struct RemovePtr<T*>
				{
					typedef T					Type;
				};
				
				template<typename T>
				struct RemovePtr<Ptr<T>>
				{
					typedef T					Type;
				};
			}

#pragma warning(push)
#pragma warning(disable:4250)
			template<typename T>
			class ValueEnumeratorWrapper : public Object, public virtual IValueEnumerator
			{
			protected:
				typedef typename trait_helper::RemovePtr<T>::Type		ContainerType;
				typedef typename ContainerType::ElementType				ElementType;

				T								wrapperPointer;
			public:
				ValueEnumeratorWrapper(const T& _wrapperPointer)
					:wrapperPointer(_wrapperPointer)
				{
				}

				Value GetCurrent()override
				{
					return BoxValue<ElementType>(wrapperPointer->Current());
				}

				vint GetIndex()override
				{
					return wrapperPointer->Index();
				}

				bool Next()override
				{
					return wrapperPointer->Next();
				}
			};

			template<typename T>
			class ValueEnumerableWrapper : public Object, public virtual IValueEnumerable
			{
			protected:
				typedef typename trait_helper::RemovePtr<T>::Type		ContainerType;
				typedef typename ContainerType::ElementType				ElementType;

				T								wrapperPointer;
			public:
				ValueEnumerableWrapper(const T& _wrapperPointer)
					:wrapperPointer(_wrapperPointer)
				{
				}

				Ptr<IValueEnumerator> CreateEnumerator()override
				{
					return new ValueEnumeratorWrapper<Ptr<collections::IEnumerator<ElementType>>>(wrapperPointer->CreateEnumerator());
				}
			};

#define WRAPPER_POINTER this->wrapperPointer

			template<typename T>
			class ValueReadonlyListWrapper : public ValueEnumerableWrapper<T>, public virtual IValueReadonlyList
			{
			protected:
				typedef typename trait_helper::RemovePtr<T>::Type		ContainerType;
				typedef typename ContainerType::ElementType				ElementType;
				typedef typename KeyType<ElementType>::Type				ElementKeyType;

			public:
				ValueReadonlyListWrapper(const T& _wrapperPointer)
					:ValueEnumerableWrapper<T>(_wrapperPointer)
				{
				}

				vint GetCount()override
				{
					return WRAPPER_POINTER->Count();
				}

				Value Get(vint index)override
				{
					return BoxValue<ElementType>(WRAPPER_POINTER->Get(index));
				}

				bool Contains(const Value& value)override
				{
					ElementKeyType item=UnboxValue<ElementKeyType>(value);
					return WRAPPER_POINTER->Contains(item);
				}

				vint IndexOf(const Value& value)override
				{
					ElementKeyType item=UnboxValue<ElementKeyType>(value);
					return WRAPPER_POINTER->IndexOf(item);
				}
			};

			template<typename T>
			class ValueArrayWrapper : public ValueReadonlyListWrapper<T>, public virtual IValueArray
			{
			protected:
				typedef typename trait_helper::RemovePtr<T>::Type		ContainerType;
				typedef typename ContainerType::ElementType				ElementType;
				typedef typename KeyType<ElementType>::Type				ElementKeyType;

			public:
				ValueArrayWrapper(const T& _wrapperPointer)
					:ValueReadonlyListWrapper<T>(_wrapperPointer)
				{
				}

				void Set(vint index, const Value& value)override
				{
					ElementType item=UnboxValue<ElementType>(value);
					WRAPPER_POINTER->Set(index, item);
				}

				void Resize(vint size)override
				{
					return WRAPPER_POINTER->Resize(size);
				}
			};

			template<typename T>
			class ValueListWrapper : public ValueReadonlyListWrapper<T>, public virtual IValueList
			{
			protected:
				typedef typename trait_helper::RemovePtr<T>::Type		ContainerType;
				typedef typename ContainerType::ElementType				ElementType;
				typedef typename KeyType<ElementType>::Type				ElementKeyType;

			public:
				ValueListWrapper(const T& _wrapperPointer)
					:ValueReadonlyListWrapper<T>(_wrapperPointer)
				{
				}

				void Set(vint index, const Value& value)override
				{
					ElementType item=UnboxValue<ElementType>(value);
					WRAPPER_POINTER->Set(index, item);
				}

				vint Add(const Value& value)override
				{
					ElementType item=UnboxValue<ElementType>(value);
					return WRAPPER_POINTER->Add(item);
				}

				vint Insert(vint index, const Value& value)override
				{
					ElementType item=UnboxValue<ElementType>(value);
					return WRAPPER_POINTER->Insert(index, item);
				}

				bool Remove(const Value& value)override
				{
					ElementKeyType item=UnboxValue<ElementKeyType>(value);
					return WRAPPER_POINTER->Remove(item);
				}

				bool RemoveAt(vint index)override
				{
					return WRAPPER_POINTER->RemoveAt(index);
				}

				void Clear()override
				{
					WRAPPER_POINTER->Clear();
				}
			};

			template<typename T, typename K>
			class ValueListWrapper<collections::Array<T, K>*> : public ValueReadonlyListWrapper<collections::Array<T, K>*>, public virtual IValueList
			{
			protected:
				typedef collections::Array<T, K>				ContainerType;
				typedef T										ElementType;
				typedef K										ElementKeyType;

			public:
				ValueListWrapper(collections::Array<T, K>* _wrapperPointer)
					:ValueReadonlyListWrapper<collections::Array<T, K>*>(_wrapperPointer)
				{
				}

				void Set(vint index, const Value& value)override
				{
					ElementType item = UnboxValue<ElementType>(value);
					WRAPPER_POINTER->Set(index, item);
				}

				vint Add(const Value& value)override
				{
					throw Exception(L"Array doesn't have Add method.");
				}

				vint Insert(vint index, const Value& value)override
				{
					throw Exception(L"Array doesn't have Insert method.");
				}

				bool Remove(const Value& value)override
				{
					throw Exception(L"Array doesn't have Remove method.");
				}

				bool RemoveAt(vint index)override
				{
					throw Exception(L"Array doesn't have RemoveAt method.");
				}

				void Clear()override
				{
					throw Exception(L"Array doesn't have Clear method.");
				}
			};

			template<typename T, typename K>
			class ValueListWrapper<collections::SortedList<T, K>*> : public ValueReadonlyListWrapper<collections::SortedList<T, K>*>, public virtual IValueList
			{
			protected:
				typedef collections::SortedList<T, K>			ContainerType;
				typedef T										ElementType;
				typedef K										ElementKeyType;

			public:
				ValueListWrapper(collections::SortedList<T, K>* _wrapperPointer)
					:ValueReadonlyListWrapper<collections::SortedList<T, K>*>(_wrapperPointer)
				{
				}

				void Set(vint index, const Value& value)override
				{
					throw Exception(L"SortedList doesn't have Set method.");
				}

				vint Add(const Value& value)override
				{
					ElementType item = UnboxValue<ElementType>(value);
					return WRAPPER_POINTER->Add(item);
				}

				vint Insert(vint index, const Value& value)override
				{
					throw Exception(L"SortedList doesn't have Insert method.");
				}

				bool Remove(const Value& value)override
				{
					ElementKeyType item = UnboxValue<ElementKeyType>(value);
					return WRAPPER_POINTER->Remove(item);
				}

				bool RemoveAt(vint index)override
				{
					return WRAPPER_POINTER->RemoveAt(index);
				}

				void Clear()override
				{
					WRAPPER_POINTER->Clear();
				}
			};

			template<typename T>
			class ValueObservableListWrapper : public ValueListWrapper<T>, public virtual IValueObservableList
			{
			public:
				ValueObservableListWrapper(const T& _wrapperPointer)
					:ValueListWrapper<T>(_wrapperPointer)
				{
				}
			};

#undef WRAPPER_POINTER

			template<typename T>
			class ValueReadonlyDictionaryWrapper : public virtual Object, public virtual IValueReadonlyDictionary
			{
			protected:
				typedef typename trait_helper::RemovePtr<T>::Type		ContainerType;
				typedef typename ContainerType::KeyContainer			KeyContainer;
				typedef typename ContainerType::ValueContainer			ValueContainer;
				typedef typename KeyContainer::ElementType				KeyValueType;
				typedef typename KeyType<KeyValueType>::Type			KeyKeyType;
				typedef typename ValueContainer::ElementType			ValueType;

				T								wrapperPointer;
				Ptr<IValueReadonlyList>			keys;
				Ptr<IValueReadonlyList>			values;
			public:
				ValueReadonlyDictionaryWrapper(const T& _wrapperPointer)
					:wrapperPointer(_wrapperPointer)
				{
				}

				Ptr<IValueReadonlyList> GetKeys()override
				{
					if(!keys)
					{
						keys=new ValueReadonlyListWrapper<const KeyContainer*>(&wrapperPointer->Keys());
					}
					return keys;
				}

				Ptr<IValueReadonlyList> GetValues()override
				{
					if(!values)
					{
						values=new ValueReadonlyListWrapper<const ValueContainer*>(&wrapperPointer->Values());
					}
					return values;
				}

				vint GetCount()override
				{
					return wrapperPointer->Count();
				}

				Value Get(const Value& key)override
				{
					KeyKeyType item=UnboxValue<KeyKeyType>(key);
					ValueType result=wrapperPointer->Get(item);
					return BoxValue<ValueType>(result);
				}
			};

#define WRAPPER_POINTER ValueReadonlyDictionaryWrapper<T>::wrapperPointer
#define KEY_VALUE_TYPE typename ValueReadonlyDictionaryWrapper<T>::KeyValueType
#define VALUE_TYPE typename ValueReadonlyDictionaryWrapper<T>::ValueType
#define KEY_KEY_TYPE typename ValueReadonlyDictionaryWrapper<T>::KeyKeyType
			
			template<typename T>
			class ValueDictionaryWrapper : public virtual ValueReadonlyDictionaryWrapper<T>, public virtual IValueDictionary
			{
			public:
				ValueDictionaryWrapper(const T& _wrapperPointer)
					:ValueReadonlyDictionaryWrapper<T>(_wrapperPointer)
				{
				}

				void Set(const Value& key, const Value& value)override
				{
					KEY_VALUE_TYPE item=UnboxValue<KEY_VALUE_TYPE>(key);
					VALUE_TYPE result=UnboxValue<VALUE_TYPE>(value);
					WRAPPER_POINTER->Set(item, result);
				}

				bool Remove(const Value& key)override
				{
					KEY_KEY_TYPE item=UnboxValue<KEY_KEY_TYPE>(key);
					return WRAPPER_POINTER->Remove(item);
				}

				void Clear()override
				{
					WRAPPER_POINTER->Clear();
				}
			};
#undef WRAPPER_POINTER
#undef KEY_VALUE_TYPE
#undef VALUE_TYPE
#undef KEY_KEY_TYPE
#pragma warning(pop)
		}
	}
}

#endif