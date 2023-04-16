/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_TYPES_TYPEDVALUESERIALIZERPROVIDER
#define VCZH_REFLECTION_TYPES_TYPEDVALUESERIALIZERPROVIDER

#include "../DescriptableInterfaces.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
/***********************************************************************
DateTime
***********************************************************************/
			
			template<>
			struct TypedValueSerializerProvider<DateTime>
			{
				static DateTime GetDefaultValue();
				static bool Serialize(const DateTime& input, WString& output);
				static bool Deserialize(const WString& input, DateTime& output);
			};
		}
	}
}

#endif
