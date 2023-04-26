/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_TYPES_TYPEDVALUESERIALIZERPROVIDER
#define VCZH_REFLECTION_TYPES_TYPEDVALUESERIALIZERPROVIDER

#include "../DescriptableInterfaces.h"
#ifdef VCZH_GCC
#include <float.h>
#endif

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
					output = i64tow(input);
					return true;
				}

				static bool Deserialize(const WString& input, T& output)
				{
					bool success = false;
					vint64_t result = wtoi64_test(input, success);
					if (!success) return false;
					if (result < MinValue || result > MaxValue) return false;
					output = (T)result;
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
					vuint64_t result = wtou64_test(input, success);
					if (!success) return false;
					if (result > MaxValue) return false;
					output = (T)result;
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

#define DEFINE_SIGNED_TVSP(TYPENAME, MINVALUE, MAXVALUE)\
		template<> struct TypedValueSerializerProvider<TYPENAME> : TypedValueSerializerProvider_Signed<TYPENAME, MINVALUE, MAXVALUE> {};\

		DEFINE_SIGNED_TVSP(vint8_t, _I8_MIN, _I8_MAX)
		DEFINE_SIGNED_TVSP(vint16_t, _I16_MIN, _I16_MAX)
		DEFINE_SIGNED_TVSP(vint32_t, _I32_MIN, _I32_MAX)
		DEFINE_SIGNED_TVSP(vint64_t, _I64_MIN, _I64_MAX)
#undef DEFINE_SIGNED_TVSP

#define DEFINE_UNSIGNED_TVSP(TYPENAME, MAXVALUE)\
		template<> struct TypedValueSerializerProvider<TYPENAME> : TypedValueSerializerProvider_Unsigned<TYPENAME, MAXVALUE> {};\

		DEFINE_UNSIGNED_TVSP(vuint8_t, _UI8_MAX)
		DEFINE_UNSIGNED_TVSP(vuint16_t, _UI16_MAX)
		DEFINE_UNSIGNED_TVSP(vuint32_t, _UI32_MAX)
		DEFINE_UNSIGNED_TVSP(vuint64_t, _UI64_MAX)
#undef DEFINE_UNSIGNED_TVSP

#define DEFINE_FLOAT_TVSP(TYPENAME, MAXVALUE)\
		template<> struct TypedValueSerializerProvider<TYPENAME> : TypedValueSerializerProvider_FloatingPoint<TYPENAME, MAXVALUE> {};\

		DEFINE_FLOAT_TVSP(float, (float)FLT_MAX)
		DEFINE_FLOAT_TVSP(double, (double)DBL_MAX)
#undef DEFINE_FLOAT_TVSP
			
#define DEFINE_TVSP(TYPENAME)\
			template<>\
			struct TypedValueSerializerProvider<TYPENAME>\
			{\
				static TYPENAME GetDefaultValue();\
				static bool Serialize(const TYPENAME& input, WString& output);\
				static bool Deserialize(const WString& input, TYPENAME& output);\
			};\

		DEFINE_TVSP(bool)
		DEFINE_TVSP(wchar_t)
		DEFINE_TVSP(WString)
		DEFINE_TVSP(Locale)
		DEFINE_TVSP(DateTime)

#undef DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER
		}
	}
}

#endif
