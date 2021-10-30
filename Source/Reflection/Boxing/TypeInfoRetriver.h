/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_BOXING_TYPEINFORETRIVER
#define VCZH_REFLECTION_BOXING_TYPEINFORETRIVER

#include "../Predefined/ObservableList.h"
#include "../Predefined/PredefinedTypes.h"
#include "../DescriptableInterfaces.h"

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
TypeInfoRetriver
***********************************************************************/

			template<typename T, TypeFlags Flag>
			struct DetailTypeInfoRetriver
			{
				static const ITypeInfo::Decorator						Decorator=ITypeInfo::TypeDescriptor;
				typedef void											Type;
				typedef void											TempValueType;
				typedef void											ResultReferenceType;
				typedef void											ResultNonReferenceType;
			};

			template<typename T>
			struct TypeInfoRetriver
			{
				static const TypeFlags															TypeFlag = TypeFlagSelector<T>::Result;
				static const TypeInfoHint														Hint = TypeHintTester<T>::Result;
				static const ITypeInfo::Decorator												Decorator = DetailTypeInfoRetriver<T, TypeFlag>::Decorator;

				typedef typename DetailTypeInfoRetriver<T, TypeFlag>::Type						Type;
				typedef typename DetailTypeInfoRetriver<T, TypeFlag>::TempValueType				TempValueType;
				typedef typename DetailTypeInfoRetriver<T, TypeFlag>::ResultReferenceType		ResultReferenceType;
				typedef typename DetailTypeInfoRetriver<T, TypeFlag>::ResultNonReferenceType	ResultNonReferenceType;

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
