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
Signed Types
***********************************************************************/

			template<typename T, T MinValue, T MaxValue>
			struct TypedValueSerializerProvider_Signed
			{
				static T GetDefaultValue()
				{
					return 0;
				}

				static bool Serialize(const T& input, WString& output)
				{
					output = u64tow(input);
					return true;
				}

				static bool Deserialize(const WString& input, T& output)
				{
					bool success = false;
					T result = wtoi64_test(input, success);
					if (!success) return false;
					if (result < MinValue || result > MaxValue) return false;
					output = (vint8_t)result;
					return true;
				}
			};

/***********************************************************************
Unsigned Types
***********************************************************************/

			template<typename T, T MaxValue>
			struct TypedValueSerializerProvider_Unsigned
			{
				static T GetDefaultValue()
				{
					return 0;
				}

				static bool Serialize(const T& input, WString& output)
				{
					output = u64tow(input);
					return true;
				}

				static bool Deserialize(const WString& input, T& output)
				{
					bool success = false;
					T result = wtou64_test(input, success);
					if (!success) return false;
					if (result > MaxValue) return false;
					output = (vuint8_t)result;
					return true;
				}
			};

/***********************************************************************
Floating Point Types
***********************************************************************/

			template<typename T, T MaxValue>
			struct TypedValueSerializerProvider_FloatingPoint
			{
				static T GetDefaultValue()
				{
					return 0;
				}

				static bool Serialize(const T& input, WString& output)
				{
					output = ftow(input);
					if (output == L"-0") output = L"0";
					return true;
				}

				static bool Deserialize(const WString& input, T& output)
				{
					bool success = false;
					double result = wtof_test(input, success);
					if (!success) return false;
					if (result < -MaxValue || result > MaxValue) return false;
					output = (T)result;
					return true;
				}
			};

/***********************************************************************
Serializable Types
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
