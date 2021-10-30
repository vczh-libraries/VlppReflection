/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_BOXING_TYPEINFORETRIVER
#define VCZH_REFLECTION_BOXING_TYPEINFORETRIVER

#include "../DescriptableInterfaces.h"
#include "../Metadata/Metadata.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
/***********************************************************************
TypeFlagTester
***********************************************************************/

			enum class TypeFlags
			{
				NonGenericType			=0,
				FunctionType			=1<<0,
				EnumerableType			=1<<1,
				ReadonlyListType		=1<<2,
				ArrayType				=1<<3,
				ListType				=1<<4,
				ObservableListType		=1<<5,
				ReadonlyDictionaryType	=1<<6,
				DictionaryType			=1<<7,
			};

			template<typename T>
			struct ValueRetriver
			{
				T* pointer;
			};

			template<typename T>
			struct ValueRetriver<T&>
			{
				T* pointer;
			};

			template<typename TDerived, TypeFlags Flag>
			struct TypeFlagTester
			{
				static const TypeFlags									Result=TypeFlags::NonGenericType;
			};

			template<typename TDerived>
			struct TypeFlagTester<TDerived, TypeFlags::FunctionType>
			{
				template<typename T>
				static void* Inherit(const Func<T>* source){ return {}; }
				static char Inherit(void* source){ return {}; }
				static char Inherit(const void* source){ return {}; }

				static const TypeFlags									Result=sizeof(Inherit(((ValueRetriver<TDerived>*)0)->pointer))==sizeof(void*)?TypeFlags::FunctionType:TypeFlags::NonGenericType;
			};

			template<typename TDerived>
			struct TypeFlagTester<TDerived, TypeFlags::EnumerableType>
			{
				template<typename T>
				static void* Inherit(const collections::LazyList<T>* source){ return {}; }
				template<typename T>
				static void* Inherit(const collections::IEnumerable<T>* source) { return {}; }
				static char Inherit(void* source){ return {}; }
				static char Inherit(const void* source){ return {}; }

				static const TypeFlags									Result=sizeof(Inherit(((ValueRetriver<TDerived>*)0)->pointer))==sizeof(void*)?TypeFlags::EnumerableType:TypeFlags::NonGenericType;
			};

			template<typename TDerived>
			struct TypeFlagTester<TDerived, TypeFlags::ReadonlyListType>
			{
				template<typename T, typename K>
				static void* Inherit(const collections::Array<T, K>* source){ return {}; }
				template<typename T, typename K>
				static void* Inherit(const collections::List<T, K>* source) { return {}; }
				template<typename T, typename K>
				static void* Inherit(const collections::SortedList<T, K>* source) { return {}; }
				template<typename T, typename K>
				static void* Inherit(const collections::ObservableListBase<T, K>* source) { return {}; }
				static char Inherit(void* source){ return {}; }
				static char Inherit(const void* source){ return {}; }

				static const TypeFlags									Result=sizeof(Inherit(((ValueRetriver<TDerived>*)0)->pointer))==sizeof(void*)?TypeFlags::ReadonlyListType:TypeFlags::NonGenericType;
			};

			template<typename TDerived>
			struct TypeFlagTester<TDerived, TypeFlags::ArrayType>
			{
				template<typename T, typename K>
				static void* Inherit(collections::Array<T, K>* source) { return {}; }
				static char Inherit(void* source) { return {}; }
				static char Inherit(const void* source) { return {}; }

				static const TypeFlags									Result = sizeof(Inherit(((ValueRetriver<TDerived>*)0)->pointer)) == sizeof(void*) ? TypeFlags::ArrayType : TypeFlags::NonGenericType;
			};

			template<typename TDerived>
			struct TypeFlagTester<TDerived, TypeFlags::ListType>
			{
				template<typename T, typename K>
				static void* Inherit(collections::List<T, K>* source) { return {}; }
				template<typename T, typename K>
				static void* Inherit(collections::ObservableListBase<T, K>* source) { return {}; }
				static char Inherit(void* source){ return {}; }
				static char Inherit(const void* source){ return {}; }

				static const TypeFlags									Result=sizeof(Inherit(((ValueRetriver<TDerived>*)0)->pointer))==sizeof(void*)?TypeFlags::ListType:TypeFlags::NonGenericType;
			};

			template<typename TDerived>
			struct TypeFlagTester<TDerived, TypeFlags::ObservableListType>
			{
				template<typename T>
				static void* Inherit(collections::ObservableList<T>* source) { return {}; }
				static char Inherit(void* source) { return {}; }
				static char Inherit(const void* source) { return {}; }

				static const TypeFlags									Result = sizeof(Inherit(((ValueRetriver<TDerived>*)0)->pointer)) == sizeof(void*) ? TypeFlags::ObservableListType : TypeFlags::NonGenericType;
			};

			template<typename TDerived>
			struct TypeFlagTester<TDerived, TypeFlags::ReadonlyDictionaryType>
			{
				template<typename K, typename V>
				static void* Inherit(const collections::Dictionary<K, V>* source){ return {}; }
				static char Inherit(void* source){ return {}; }
				static char Inherit(const void* source){ return {}; }

				static const TypeFlags									Result=sizeof(Inherit(((ValueRetriver<TDerived>*)0)->pointer))==sizeof(void*)?TypeFlags::ReadonlyDictionaryType:TypeFlags::NonGenericType;
			};

			template<typename TDerived>
			struct TypeFlagTester<TDerived, TypeFlags::DictionaryType>
			{
				template<typename K, typename V>
				static void* Inherit(collections::Dictionary<K, V>* source){ return {}; }
				static char Inherit(void* source){ return {}; }
				static char Inherit(const void* source){ return {}; }

				static const TypeFlags									Result=sizeof(Inherit(((ValueRetriver<TDerived>*)0)->pointer))==sizeof(void*)?TypeFlags::DictionaryType:TypeFlags::NonGenericType;
			};

/***********************************************************************
TypeFlagSelector
***********************************************************************/

			template<typename T, TypeFlags Flag>
			struct TypeFlagSelectorCase
			{
				static const  TypeFlags									Result=TypeFlags::NonGenericType;
			};

			template<typename T>
			struct TypeFlagSelectorCase<T, (TypeFlags)((vint)TypeFlags::FunctionType)>
			{
				static const  TypeFlags									Result=TypeFlags::FunctionType;
			};

			template<typename T>
			struct TypeFlagSelectorCase<T, (TypeFlags)((vint)TypeFlags::EnumerableType)>
			{
				static const  TypeFlags									Result=TypeFlags::EnumerableType;
			};

			template<typename T>
			struct TypeFlagSelectorCase<T, (TypeFlags)((vint)TypeFlags::EnumerableType|(vint)TypeFlags::ReadonlyListType)>
			{
				static const  TypeFlags									Result = TypeFlags::ReadonlyListType;
			};

			template<typename T>
			struct TypeFlagSelectorCase<T, (TypeFlags)((vint)TypeFlags::EnumerableType|(vint)TypeFlags::ReadonlyListType|(vint)TypeFlags::ArrayType)>
			{
				static const  TypeFlags									Result = TypeFlags::ArrayType;
			};

			template<typename T>
			struct TypeFlagSelectorCase<T, (TypeFlags)((vint)TypeFlags::EnumerableType|(vint)TypeFlags::ReadonlyListType|(vint)TypeFlags::ListType)>
			{
				static const  TypeFlags									Result = TypeFlags::ListType;
			};

			template<typename T>
			struct TypeFlagSelectorCase<T, (TypeFlags)((vint)TypeFlags::EnumerableType|(vint)TypeFlags::ReadonlyListType|(vint)TypeFlags::ListType|(vint)TypeFlags::ObservableListType)>
			{
				static const  TypeFlags									Result = TypeFlags::ObservableListType;
			};

			template<typename T>
			struct TypeFlagSelectorCase<T, (TypeFlags)((vint)TypeFlags::EnumerableType|(vint)TypeFlags::ReadonlyDictionaryType)>
			{
				static const  TypeFlags									Result=TypeFlags::ReadonlyDictionaryType;
			};

			template<typename T>
			struct TypeFlagSelectorCase<T, (TypeFlags)((vint)TypeFlags::EnumerableType |(vint)TypeFlags::ReadonlyDictionaryType |(vint)TypeFlags::DictionaryType)>
			{
				static const  TypeFlags									Result=TypeFlags::DictionaryType;
			};

			template<typename T>
			struct TypeFlagSelector
			{
				static const TypeFlags									Result =
					TypeFlagSelectorCase<
					T, 
					(TypeFlags)
					( (vint)TypeFlagTester<T, TypeFlags::FunctionType>::Result
					| (vint)TypeFlagTester<T, TypeFlags::EnumerableType>::Result
					| (vint)TypeFlagTester<T, TypeFlags::ReadonlyListType>::Result
					| (vint)TypeFlagTester<T, TypeFlags::ArrayType>::Result
					| (vint)TypeFlagTester<T, TypeFlags::ListType>::Result
					| (vint)TypeFlagTester<T, TypeFlags::ObservableListType>::Result
					| (vint)TypeFlagTester<T, TypeFlags::ReadonlyDictionaryType>::Result
					| (vint)TypeFlagTester<T, TypeFlags::DictionaryType>::Result
					)
					>::Result;
			};

/***********************************************************************
TypeHintTester
***********************************************************************/

			template<typename T>
			struct TypeHintTester
			{
				static const TypeInfoHint								Result = TypeInfoHint::Normal;
			};

			template<TypeFlags Flags>
			struct TypeHintTesterForReference
			{
				static const TypeInfoHint								Result = TypeInfoHint::NativeCollectionReference;
			};

			template<>
			struct TypeHintTesterForReference<TypeFlags::NonGenericType>
			{
				static const TypeInfoHint								Result = TypeInfoHint::Normal;
			};

			template<>
			struct TypeHintTesterForReference<TypeFlags::FunctionType>
			{
				static const TypeInfoHint								Result = TypeInfoHint::Normal;
			};

			template<typename T>
			struct TypeHintTester<T*>
			{
				static const TypeInfoHint								Result = TypeHintTester<T>::Result;
			};

			template<typename T>
			struct TypeHintTester<T&>
			{
				static const TypeInfoHint								Result = TypeHintTester<T>::Result == TypeInfoHint::Normal
																					? TypeHintTesterForReference<TypeFlagSelector<T&>::Result>::Result
																					: TypeHintTester<T>::Result
																					;
			};

			template<typename T>
			struct TypeHintTester<const T>
			{
				static const TypeInfoHint								Result = TypeHintTester<T>::Result;
			};

			template<typename T>
			struct TypeHintTester<collections::LazyList<T>>
			{
				static const TypeInfoHint								Result = TypeInfoHint::LazyList;
			};

			template<typename T>
			struct TypeHintTester<collections::Array<T>>
			{
				static const TypeInfoHint								Result = TypeInfoHint::Array;
			};

			template<typename T>
			struct TypeHintTester<collections::List<T>>
			{
				static const TypeInfoHint								Result = TypeInfoHint::List;
			};

			template<typename T>
			struct TypeHintTester<collections::SortedList<T>>
			{
				static const TypeInfoHint								Result = TypeInfoHint::SortedList;
			};

			template<typename T>
			struct TypeHintTester<collections::ObservableList<T>>
			{
				static const TypeInfoHint								Result = TypeInfoHint::ObservableList;
			};

			template<typename K, typename V>
			struct TypeHintTester<collections::Dictionary<K, V>>
			{
				static const TypeInfoHint								Result = TypeInfoHint::Dictionary;
			};

/***********************************************************************
DetailTypeInfoRetriver
***********************************************************************/

			template<typename T, TypeFlags Flag>
			struct DetailTypeInfoRetriver;

/***********************************************************************
Basic Types
***********************************************************************/

			template<typename T>
			struct DetailTypeInfoRetriver<T, TypeFlags::NonGenericType>
			{
				static const ITypeInfo::Decorator						Decorator=ITypeInfo::TypeDescriptor;

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					return MakePtr<TypeDescriptorTypeInfo>(GetTypeDescriptor<T>(), hint);
				}
#endif
			};

/***********************************************************************
Decorated Types
***********************************************************************/

			template<typename T>
			struct DetailTypeInfoRetriver<T*, TypeFlags::NonGenericType>
			{
				typedef DetailTypeInfoRetriver<T, TypeFlags::NonGenericType>	UpLevelRetriver;

				static const ITypeInfo::Decorator								Decorator=ITypeInfo::RawPtr;

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

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					return MakePtr<NullableTypeInfo>(TypeInfoRetriver<T>::CreateTypeInfo());
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

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					auto arrayType = MakePtr<TypeDescriptorTypeInfo>(GetTypeDescriptor<TCollectionType>(), hint);

					auto genericType = MakePtr<GenericTypeInfo>(arrayType);
					genericType->AddGenericArgument(TypeInfoRetriver<typename T::ElementType>::CreateTypeInfo());

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

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					auto arrayType = MakePtr<TypeDescriptorTypeInfo>(GetTypeDescriptor<TCollectionType>(), hint);

					auto genericType = MakePtr<GenericTypeInfo>(arrayType);
					genericType->AddGenericArgument(TypeInfoRetriver<typename T::KeyContainer::ElementType>::CreateTypeInfo());
					genericType->AddGenericArgument(TypeInfoRetriver<typename T::ValueContainer::ElementType>::CreateTypeInfo());

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

/***********************************************************************
TypeInfoRetriver
***********************************************************************/

			template<typename T>
			struct TypeInfoRetriver
			{
				static const TypeFlags												TypeFlag = TypeFlagSelector<T>::Result;
				static const TypeInfoHint											Hint = TypeHintTester<T>::Result;
				typedef DetailTypeInfoRetriver<std::remove_cvref_t<T>, TypeFlag>	Detail;
				static const ITypeInfo::Decorator									Decorator = Detail::Decorator;

#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo()
				{
					return DetailTypeInfoRetriver<std::remove_cvref_t<T>, TypeFlag>::CreateTypeInfo(Hint);
				}
#endif
			};

			template<>
			struct TypeInfoRetriver<void> : public TypeInfoRetriver<VoidValue>
			{
			};
		}
	}
}

#endif
