#include "../../Source/Reflection/GuiTypeDescriptorReflection.h"
#include <limits>
#include <float.h>

using namespace vl;
using namespace vl::reflection::description;

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace test
{
	template<typename T, typename TValue, vint Count, WString(*ToString)(TValue)>
	void TestNumber(TValue(&values)[Count], TValue min, TValue max, bool testGetText)
	{
		ITypeDescriptor* type = GetTypeDescriptor<T>();
		auto serializer = type->GetSerializableType();
		TEST_CASE(L"Ensure ITypeDescriptor")
		{
			TEST_ASSERT(type);
			TEST_ASSERT(serializer);
		});

		for (auto i : values)
		{
			TEST_CASE(L"Test number: " + ToString(i))
			{
				if (min <= i && i <= max)
				{
					Value value;
					{
						TEST_ASSERT(serializer->Deserialize(ToString(i), value));
						TEST_ASSERT(value.GetValueType() == Value::BoxedValue);
#ifndef VCZH_DEBUG_METAONLY_REFLECTION
						TEST_ASSERT(value.GetTypeDescriptor() == type);
#endif
						TEST_ASSERT(value.GetRawPtr() == 0);
						TEST_ASSERT(value.GetSharedPtr().Obj() == 0);

						WString output;
						TEST_ASSERT(serializer->Serialize(value, output));
						if (testGetText)
						{
							TEST_ASSERT(output == ToString(i));
						}
					}
					{
						T n = UnboxValue<T>(value);
#ifdef VCZH_MSVC
						TEST_ASSERT(n == (T)i);
#endif
					}
				}
				else
				{
					Value value;
					{
						TEST_ASSERT(serializer->Deserialize(ToString(i), value) == false);
						TEST_ASSERT(value.GetValueType() == Value::Null);
#ifndef VCZH_DEBUG_METAONLY_REFLECTION
						TEST_ASSERT(value.GetTypeDescriptor() == 0);
#endif
						TEST_ASSERT(value.GetRawPtr() == 0);
						TEST_ASSERT(value.GetSharedPtr().Obj() == 0);
					}
				}
			});
		}
	}

	template<typename T, vint LegalCount, vint IllegalCount>
	void TestLiteral(WString legalsText[], WString illegalsText[], T legals[])
	{
		ITypeDescriptor* type = GetTypeDescriptor<T>();
		auto serializer = type->GetSerializableType();
		TEST_CASE(L"Ensure ITypeDescriptor")
		{
			TEST_ASSERT(type);
			TEST_ASSERT(serializer);
		});

		for (vint x = 0; x < LegalCount; x++)
		{
			WString i = legalsText[x];
			TEST_CASE(L"Test legal literal:" + i)
			{
				T j = legals[x];
				Value value;
				{
					TEST_ASSERT(serializer->Deserialize(i, value));
					TEST_ASSERT(value.GetValueType() == Value::BoxedValue);
#ifndef VCZH_DEBUG_METAONLY_REFLECTION
					TEST_ASSERT(value.GetTypeDescriptor() == type);
#endif
					TEST_ASSERT(value.GetRawPtr() == 0);
					TEST_ASSERT(value.GetSharedPtr().Obj() == 0);
				}
				{
					WString output;
					TEST_ASSERT(serializer->Serialize(value, output));
					TEST_ASSERT(i == output);
				}
				{
					T n = UnboxValue<T>(value);
					TEST_ASSERT(n == j);
				}
			});
		}

		for (vint x = 0; x < IllegalCount; x++)
		{
			WString i = illegalsText[x];
			TEST_CASE(L"Test illegal literal:" + i)
			{
				Value value;
				TEST_ASSERT(serializer->Deserialize(i, value) == false);
				TEST_ASSERT(value.GetValueType() == Value::Null);
#ifndef VCZH_DEBUG_METAONLY_REFLECTION
				TEST_ASSERT(value.GetTypeDescriptor() == 0);
#endif
				TEST_ASSERT(value.GetRawPtr() == 0);
				TEST_ASSERT(value.GetSharedPtr().Obj() == 0);
			});
		}
	}

	template<typename T>
	void TestInt()
	{
		vint64_t values[]=
		{
			_I64_MIN,
			_I64_MIN+1,
			_I32_MIN,
			_I32_MIN+1,
			_I16_MIN,
			_I16_MIN+1,
			_I8_MIN,
			_I8_MIN+1,
			-1,
			0,
			1,
			_I8_MAX-1,
			_I8_MAX,
			_I16_MAX-1,
			_I16_MAX,
			_I32_MAX-1,
			_I32_MAX,
			_I64_MAX-1,
			_I64_MAX,
		};

		TEST_CATEGORY(L"Test signed integers")
		{
			TestNumber<T, vint64_t, sizeof(values) / sizeof(*values), &i64tow>(values, std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), true);
		});
	}
	
	template<typename T>
	void TestUInt()
	{
		vuint64_t values[]=
		{
			0,
			1,
			_UI8_MAX-1,
			_UI8_MAX,
			_UI16_MAX-1,
			_UI16_MAX,
			_UI32_MAX-1,
			_UI32_MAX,
			_UI64_MAX-1,
			_UI64_MAX,
		};

		TEST_CATEGORY(L"Test unsigned integers")
		{
			TestNumber<T, vuint64_t, sizeof(values) / sizeof(*values), &u64tow>(values, std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), true);
		});
	}

	template<typename T>
	void TestFloat()
	{
		double values[]=
		{
			-DBL_MAX,
			-FLT_MAX,
			-FLT_MIN,
			-DBL_MIN,
			-1,
			0,
			1,
			DBL_MIN,
			FLT_MIN,
			FLT_MAX,
			DBL_MAX,
		};

		TEST_CATEGORY(L"Test floats")
		{
			TestNumber<T, double, sizeof(values) / sizeof(*values), &ftow>(values, -std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), false);
		});
	}

	void TestBool()
	{
		WString legalsText[]={L"true", L"false"};
		WString illegalsText[]={L"TRUE", L"True", L"FALSE", L"False", L""};
		bool legals[]={true, false};

		TEST_CATEGORY(L"Test booleans")
		{
			TestLiteral<bool, sizeof(legalsText) / sizeof(*legalsText), sizeof(illegalsText) / sizeof(*illegalsText)>(legalsText, illegalsText, legals);
		});
	}

	void TestString()
	{
		WString legals[]={L"a", L"b", L"c"};

		TEST_CATEGORY(L"Test strings")
		{
			TestLiteral<WString, sizeof(legals) / sizeof(*legals), 0>(legals, 0, legals);
		});
	}

	void TestDateTime()
	{
		WString legalsText[]={L"2000-01-01 00:00:00.000", L"2012-12-21 13:30:45.123", L"2013-06-11 14:30:00.000"};
		WString illegalsText[]={L"2000-01-01", L"00:00:00.000", L"2000-01-01 00:00:00"};
		DateTime legals[]={DateTime::FromDateTime(2000, 1, 1), DateTime::FromDateTime(2012, 12, 21, 13, 30, 45, 123), DateTime::FromDateTime(2013, 6, 11, 14, 30, 0)};

		TEST_CATEGORY(L"Test datetime")
		{
			TestLiteral<DateTime, sizeof(legalsText) / sizeof(*legalsText), sizeof(illegalsText) / sizeof(*illegalsText)>(legalsText, illegalsText, legals);
		});
	}

	void TestLocale()
	{
		WString legalsText[]={L"zh-CN", L"en-US"};
		Locale legals[]={Locale(L"zh-CN"), Locale(L"en-US")};

		TEST_CATEGORY(L"Test locales")
		{
			TestLiteral<Locale, sizeof(legalsText) / sizeof(*legalsText), 0>(legalsText, 0, legals);
		});
	}
}
using namespace test;

TEST_FILE
{
	TEST_CATEGORY(L"Predefined types")
	{
#ifndef VCZH_DEBUG_METAONLY_REFLECTION
		TEST_CASE_ASSERT(LoadPredefinedTypes());
#endif
		TestInt<vint8_t>();
		TestInt<vint16_t>();
		TestInt<vint32_t>();
		TestInt<vint64_t>();
		TestUInt<vuint8_t>();
		TestUInt<vuint16_t>();
		TestUInt<vuint32_t>();
		TestUInt<vuint64_t>();
		TestFloat<float>();
		TestFloat<double>();
		TestBool();
		TestString();
		TestDateTime();
		TestLocale();
		TEST_CASE_ASSERT(ResetGlobalTypeManager());
	});
}

#endif