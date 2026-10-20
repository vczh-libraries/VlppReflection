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

			template<typename T>
			T CopyVariantOption(const T& value)
			{
				if constexpr (std::is_copy_constructible_v<T>)
				{
					return T(value);
				}
				else
				{
					T result;
					collections::CopyFrom(result, value);
					return result;
				}
			}

			template<typename T>
			struct StaticVariantBox
			{
				T							value;

				StaticVariantBox(const T& _value)
					:value(CopyVariantOption(_value))
				{
				}

				StaticVariantBox(const StaticVariantBox<T>& box)
					:value(CopyVariantOption(box.value))
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
