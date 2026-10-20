/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_WRAPPERS_VARIANTWRAPPERS
#define VCZH_REFLECTION_WRAPPERS_VARIANTWRAPPERS

#include "../Boxing/BoxingParameter.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
/***********************************************************************
Variant Wrappers
***********************************************************************/

			// Compare extracted payloads with numeric promotion in every reflection mode.
			// Ordinary Value comparison retains its existing mode-dependent behavior.
			extern std::partial_ordering CompareVariantPayloads(const Value& a, const Value& b);

			template<typename T>
			constexpr bool IsVariantNativeCollection =
				TypeInfoRetriver<T>::TypeFlag == TypeFlags::ReadonlyListType ||
				TypeInfoRetriver<T>::TypeFlag == TypeFlags::ArrayType ||
				TypeInfoRetriver<T>::TypeFlag == TypeFlags::ListType ||
				TypeInfoRetriver<T>::TypeFlag == TypeFlags::ObservableListType ||
				TypeInfoRetriver<T>::TypeFlag == TypeFlags::ReadonlyDictionaryType ||
				TypeInfoRetriver<T>::TypeFlag == TypeFlags::DictionaryType;

			template<typename T>
			struct StaticVariantBox
			{
				T							value;

				StaticVariantBox(const T& _value)
					requires(!IsVariantNativeCollection<T>)
					:value(_value)
				{
				}

				StaticVariantBox(const T& _value)
					requires(IsVariantNativeCollection<T>)
				{
					collections::CopyFrom(value, _value);
				}

				StaticVariantBox(const StaticVariantBox<T>& box)
					:StaticVariantBox(box.value)
				{
				}

				std::partial_ordering operator<=>(const StaticVariantBox<T>& box)const
				{
					if constexpr (TypeInfoRetriver<T>::TypeFlag == TypeFlags::FunctionType)
					{
						return std::partial_ordering::unordered;
					}
					else if constexpr (requires { value <=> box.value; })
					{
						return value <=> box.value;
					}
					else
					{
						return std::partial_ordering::unordered;
					}
				}

				bool operator==(const StaticVariantBox<T>& box)const
				{
					return (*this <=> box) == 0;
				}
			};

#ifndef VCZH_DEBUG_NO_REFLECTION
			struct DynamicVariantBox
			{
				Value						value;
				Ptr<ITypeInfo>				type;

				std::partial_ordering operator<=>(const DynamicVariantBox& box)const
				{
					return value <=> box.value;
				}

				bool operator==(const DynamicVariantBox& box)const
				{
					return (*this <=> box) == 0;
				}
			};
#endif
		}
	}
}

#endif
