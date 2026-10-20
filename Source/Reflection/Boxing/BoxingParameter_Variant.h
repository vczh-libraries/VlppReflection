/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_BOXING_BOXINGPARAMETER_VARIANT
#define VCZH_REFLECTION_BOXING_BOXINGPARAMETER_VARIANT

#include "BoxingParameter_Containers.h"
#include "BoxingParameter_Functions.h"
#include "../Wrappers/VariantWrappers.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
/***********************************************************************
Variants
***********************************************************************/

			template<typename T>
			Value BoxVariantOption(const T& value)
			{
				constexpr auto flag = TypeInfoRetriver<T>::TypeFlag;
				if constexpr (IsVariantNativeCollection<T>)
				{
					auto copy = Ptr(new T);
					collections::CopyFrom(*copy.Obj(), value);
					if constexpr (flag == TypeFlags::ReadonlyListType)
					{
						return Value::From(Ptr(new ValueReadonlyListWrapper<Ptr<T>>(copy)));
					}
					else if constexpr (flag == TypeFlags::ArrayType)
					{
						return Value::From(Ptr(new ValueArrayWrapper<Ptr<T>>(copy)));
					}
					else if constexpr (flag == TypeFlags::ListType)
					{
						return Value::From(Ptr(new ValueListWrapper<Ptr<T>>(copy)));
					}
					else if constexpr (flag == TypeFlags::ObservableListType)
					{
						return Value::From(Ptr(new ValueObservableListWrapper<Ptr<T>>(copy)));
					}
					else if constexpr (flag == TypeFlags::ReadonlyDictionaryType)
					{
						return Value::From(Ptr(new ValueReadonlyDictionaryWrapper<Ptr<T>>(copy)));
					}
					else
					{
						return Value::From(Ptr(new ValueDictionaryWrapper<Ptr<T>>(copy)));
					}
				}
				else
				{
					return description::BoxParameter(value);
				}
			}

			template<typename ...T>
			struct ParameterAccessor<Variant<T...>, TypeFlags::VariantType>
			{
				using VariantType = Variant<T...>;

				template<typename TOption>
				static VariantType* CopyOption(const TOption& option)
				{
					if constexpr (!IsVariantNativeCollection<TOption>)
					{
						return new VariantType(option);
					}
					else
					{
						auto result = new VariantType(VariantIndex<VariantType::template IndexOf<TOption>>{});
						collections::CopyFrom(result->template Get<TOption>(), option);
						return result;
					}
				}

				static Value BoxParameter(const VariantType& object, ITypeDescriptor* typeDescriptor)
				{
					Value result;
					object.Apply([&]<typename TOption>(const TOption& option)
					{
#ifdef VCZH_DEBUG_NO_REFLECTION
						using Box = IValueType::TypedBox<StaticVariantBox<TOption>>;
						result = Value::From(Ptr(new Box(StaticVariantBox<TOption>(option))), nullptr);
#else
						DynamicVariantBox box;
						box.type = TypeInfoRetriver<TOption>::CreateTypeInfo();
						box.value = BoxVariantOption(option);
						result = Value::From(Ptr(new IValueType::TypedBox<DynamicVariantBox>(box)), nullptr);
#endif
					});
					return result;
				}

				static Unboxed<VariantType> UnboxParameter(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					VariantType* result = nullptr;
					auto tryOption = [&]<typename TOption>()
					{
#ifdef VCZH_DEBUG_NO_REFLECTION
						if (auto box = value.GetBoxedValue().Cast<IValueType::TypedBox<StaticVariantBox<TOption>>>())
						{
							result = CopyOption(box->value.value);
							return true;
						}
#else
						if (auto box = value.GetBoxedValue().Cast<IValueType::TypedBox<DynamicVariantBox>>())
						{
							auto type = TypeInfoRetriver<TOption>::CreateTypeInfo();
							if (IsSameTypeInfo(type.Obj(), box->value.type.Obj()))
							{
								auto option = description::UnboxParameter<TOption>(box->value.value);
								result = CopyOption(option.Ref());
								return true;
							}
						}
#endif
						return false;
					};
					if (!(tryOption.template operator()<T>() || ...))
					{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
						auto type = TypeInfoRetriver<VariantType>::CreateTypeInfo();
						throw ArgumentTypeMismtatchException(valueName, type.Obj(), value);
#else
						CHECK_FAIL(L"vl::reflection::description::UnboxParameter()#Argument type mismatch.");
#endif
					}
					return { result, true };
				}
			};

			template<typename ...T>
			struct ParameterAccessor<const Variant<T...>, TypeFlags::VariantType> : ParameterAccessor<Variant<T...>, TypeFlags::VariantType>
			{
			};
		}
	}
}

#endif
