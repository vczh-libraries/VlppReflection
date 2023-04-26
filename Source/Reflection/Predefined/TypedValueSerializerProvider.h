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
Constants
***********************************************************************/

			template<typename T>
			struct TypedValueSerializerMinMax;

			template<>
			struct TypedValueSerializerMinMax<vint8_t>
			{
				static constexpr vint8_t Min = _I8_MIN;
				static constexpr vint8_t Max = _I8_MAX;
			};

			template<>
			struct TypedValueSerializerMinMax<vint16_t>
			{
				static constexpr vint16_t Min = _I16_MIN;
				static constexpr vint16_t Max = _I16_MAX;
			};

			template<>
			struct TypedValueSerializerMinMax<vint32_t>
			{
				static constexpr vint32_t Min = _I32_MIN;
				static constexpr vint32_t Max = _I32_MAX;
			};

			template<>
			struct TypedValueSerializerMinMax<vint64_t>
			{
				static constexpr vint64_t Min = _I64_MIN;
				static constexpr vint64_t Max = _I64_MAX;
			};

			template<>
			struct TypedValueSerializerMinMax<vuint8_t>
			{
				static constexpr vuint8_t Min = 0;
				static constexpr vuint8_t Max = _UI8_MAX;
			};

			template<>
			struct TypedValueSerializerMinMax<vuint16_t>
			{
				static constexpr vuint16_t Min = 0;
				static constexpr vuint16_t Max = _UI16_MAX;
			};

			template<>
			struct TypedValueSerializerMinMax<vuint32_t>
			{
				static constexpr vuint32_t Min = 0;
				static constexpr vuint32_t Max = _UI32_MAX;
			};

			template<>
			struct TypedValueSerializerMinMax<vuint64_t>
			{
				static constexpr vuint64_t Min = 0;
				static constexpr vuint64_t Max = _UI64_MAX;
			};

			template<>
			struct TypedValueSerializerMinMax<float>
			{
				static constexpr float Min = (float)-FLT_MAX;
				static constexpr float Max = (float)FLT_MAX;
			};

			template<>
			struct TypedValueSerializerMinMax<double>
			{
				static constexpr double Min = (double)-DBL_MAX;
				static constexpr double Max = (double)DBL_MAX;
			};

/***********************************************************************
Signed Types
***********************************************************************/

			template<typename T>
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
					constexpr T MinValue = TypedValueSerializerMinMax<T>::Min;
					constexpr T MaxValue = TypedValueSerializerMinMax<T>::Max;
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

			template<typename T>
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
					constexpr T MaxValue = TypedValueSerializerMinMax<T>::Max;
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

			template<typename T>
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
					constexpr T MinValue = TypedValueSerializerMinMax<T>::Min;
					constexpr T MaxValue = TypedValueSerializerMinMax<T>::Max;
					bool success = false;
					double result = wtof_test(input, success);
					if (!success) return false;
					if (result < MinValue || result > MaxValue) return false;
					output = (T)result;
					return true;
				}
			};

/***********************************************************************
Serializable Types
***********************************************************************/

#define DEFINE_SIGNED_TVSP(TYPENAME)\
		template<> struct TypedValueSerializerProvider<TYPENAME> : TypedValueSerializerProvider_Signed<TYPENAME> {};\

		DEFINE_SIGNED_TVSP(vint8_t)
		DEFINE_SIGNED_TVSP(vint16_t)
		DEFINE_SIGNED_TVSP(vint32_t)
		DEFINE_SIGNED_TVSP(vint64_t)
#undef DEFINE_SIGNED_TVSP

#define DEFINE_UNSIGNED_TVSP(TYPENAME)\
		template<> struct TypedValueSerializerProvider<TYPENAME> : TypedValueSerializerProvider_Unsigned<TYPENAME> {};\

		DEFINE_UNSIGNED_TVSP(vuint8_t)
		DEFINE_UNSIGNED_TVSP(vuint16_t)
		DEFINE_UNSIGNED_TVSP(vuint32_t)
		DEFINE_UNSIGNED_TVSP(vuint64_t)
#undef DEFINE_UNSIGNED_TVSP

#define DEFINE_FLOAT_TVSP(TYPENAME)\
		template<> struct TypedValueSerializerProvider<TYPENAME> : TypedValueSerializerProvider_FloatingPoint<TYPENAME> {};\

		DEFINE_FLOAT_TVSP(float)
		DEFINE_FLOAT_TVSP(double)
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
