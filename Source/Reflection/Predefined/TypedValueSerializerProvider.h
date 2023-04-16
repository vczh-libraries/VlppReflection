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
			
#define DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(TYPENAME)\
			template<>\
			struct TypedValueSerializerProvider<TYPENAME>\
			{\
				static TYPENAME GetDefaultValue();\
				static bool Serialize(const TYPENAME& input, WString& output);\
				static bool Deserialize(const WString& input, TYPENAME& output);\
			};\

		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(vuint8_t)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(vuint16_t)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(vuint32_t)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(vuint64_t)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(vint8_t)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(vint16_t)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(vint32_t)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(vint64_t)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(float)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(double)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(wchar_t)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(bool)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(WString)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(Locale)
		DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(DateTime)

#undef DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER
		}
	}
}

#endif
