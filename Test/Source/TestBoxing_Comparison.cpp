#include "Common.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection;
using namespace vl::reflection::description;

namespace TestReflection_TestObjects
{
	struct ValueToCompare
	{
		WString a;
		vint b;

		std::strong_ordering operator<=>(const ValueToCompare&) const = default;
		bool operator==(const ValueToCompare&) const = default;
	};
}
using namespace TestReflection_TestObjects;

#define _ ,

#define TYPE_LIST(F)\
	F(ValueToCompare)\

BEGIN_TYPE_INFO_NAMESPACE

	TYPE_LIST(DECL_TYPE_INFO)
	TYPE_LIST(IMPL_CPP_TYPE_INFO)

	BEGIN_STRUCT_MEMBER(ValueToCompare)
		STRUCT_MEMBER(a)
		STRUCT_MEMBER(b)
	END_STRUCT_MEMBER(ValueToCompare)

	class TestTypeLoader_Comparison : public Object, public ITypeLoader
	{
	public:
		void Load(ITypeManager* manager)override
		{
			TYPE_LIST(ADD_TYPE_INFO)
		}
		
		void Unload(ITypeManager* manager)override
		{
		}
	};

END_TYPE_INFO_NAMESPACE

#undef TYPE_LIST

namespace reflection_test
{
	template<typename T>
	void TestComparison(T a, T b)
	{
		{
			auto x = a < b;
			auto y = BoxValue<T>(a) < BoxValue<T>(b);
			TEST_ASSERT(x == y);
		}
		{
			auto x = a <= b;
			auto y = BoxValue<T>(a) <= BoxValue<T>(b);
			TEST_ASSERT(x == y);
		}
		{
			auto x = a > b;
			auto y = BoxValue<T>(a) > BoxValue<T>(b);
			TEST_ASSERT(x == y);
		}
		{
			auto x = a >= b;
			auto y = BoxValue<T>(a) >= BoxValue<T>(b);
			TEST_ASSERT(x == y);
		}
		{
			auto x = a == b;
			auto y = BoxValue<T>(a) == BoxValue<T>(b);
			TEST_ASSERT(x == y);
		}
		{
			auto x = a != b;
			auto y = BoxValue<T>(a) != BoxValue<T>(b);
			TEST_ASSERT(x == y);
		}
	}

	template<typename T>
	void TestCompareInt()
	{
		unittest::UnitTest::PrintMessage(L"0 <=> 0", unittest::UnitTest::MessageKind::Info);
		TestComparison<T>(0, 0);
		unittest::UnitTest::PrintMessage(L"0 <=> 1", unittest::UnitTest::MessageKind::Info);
		TestComparison<T>(0, 1);
		unittest::UnitTest::PrintMessage(L"1 <=> 0", unittest::UnitTest::MessageKind::Info);
		TestComparison<T>(1, 0);
	}

	template<typename T>
	void TestCompareFloat()
	{
		unittest::UnitTest::PrintMessage(L"0 <=> 0", unittest::UnitTest::MessageKind::Info);
		TestComparison<T>(0, 0);
		unittest::UnitTest::PrintMessage(L"0 <=> 1", unittest::UnitTest::MessageKind::Info);
		TestComparison<T>(0, 1);
		unittest::UnitTest::PrintMessage(L"1 <=> 0", unittest::UnitTest::MessageKind::Info);
		TestComparison<T>(1, 0);
	}

	void TestCompareBool()
	{
		unittest::UnitTest::PrintMessage(L"false <=> false", unittest::UnitTest::MessageKind::Info);
		TestComparison<bool>(false, false);
		unittest::UnitTest::PrintMessage(L"false <=> true", unittest::UnitTest::MessageKind::Info);
		TestComparison<bool>(false, true);
		unittest::UnitTest::PrintMessage(L"true <=> false", unittest::UnitTest::MessageKind::Info);
		TestComparison<bool>(true, false);
	}

	void TestCompareString()
	{
		auto a = WString::Unmanaged(L"a");
		auto b = WString::Unmanaged(L"b");
		auto ab = WString::Unmanaged(L"ab");

		unittest::UnitTest::PrintMessage(L"a <=> a", unittest::UnitTest::MessageKind::Info);
		TestComparison<WString>(a, a);
		unittest::UnitTest::PrintMessage(L"a <=> b", unittest::UnitTest::MessageKind::Info);
		TestComparison<WString>(a, b);
		unittest::UnitTest::PrintMessage(L"b <=> a", unittest::UnitTest::MessageKind::Info);
		TestComparison<WString>(b, a);
		unittest::UnitTest::PrintMessage(L"a <=> ab", unittest::UnitTest::MessageKind::Info);
		TestComparison<WString>(a, ab);
		unittest::UnitTest::PrintMessage(L"ab <=> a", unittest::UnitTest::MessageKind::Info);
		TestComparison<WString>(ab, a);
	}

	void TestCompareLocale()
	{
		Locale a(L"a");
		Locale b(L"b");
		Locale ab(L"ab");

		unittest::UnitTest::PrintMessage(L"a <=> a", unittest::UnitTest::MessageKind::Info);
		TestComparison<Locale>(a, a);
		unittest::UnitTest::PrintMessage(L"a <=> b", unittest::UnitTest::MessageKind::Info);
		TestComparison<Locale>(a, b);
		unittest::UnitTest::PrintMessage(L"b <=> a", unittest::UnitTest::MessageKind::Info);
		TestComparison<Locale>(b, a);
		unittest::UnitTest::PrintMessage(L"a <=> ab", unittest::UnitTest::MessageKind::Info);
		TestComparison<Locale>(a, ab);
		unittest::UnitTest::PrintMessage(L"ab <=> a", unittest::UnitTest::MessageKind::Info);
		TestComparison<Locale>(ab, a);
	}

	void TestCompareStruct()
	{
		ValueToCompare a1 = { L"a",1 };
		ValueToCompare a2 = { L"a",2 };
		ValueToCompare b2 = { L"b",2 };

		unittest::UnitTest::PrintMessage(L"a1 <=> a1", unittest::UnitTest::MessageKind::Info);
		TestComparison<ValueToCompare>(a1, a1);
		unittest::UnitTest::PrintMessage(L"a1 <=> b2", unittest::UnitTest::MessageKind::Info);
		TestComparison<ValueToCompare>(a1, b2);
		unittest::UnitTest::PrintMessage(L"b2 <=> a1", unittest::UnitTest::MessageKind::Info);
		TestComparison<ValueToCompare>(b2, a1);
		unittest::UnitTest::PrintMessage(L"a1 <=> a2", unittest::UnitTest::MessageKind::Info);
		TestComparison<ValueToCompare>(a1, a2);
		unittest::UnitTest::PrintMessage(L"a2 <=> a1", unittest::UnitTest::MessageKind::Info);
		TestComparison<ValueToCompare>(a2, a1);
	}
}
using namespace reflection_test;

#define TEST_CASE_REFLECTION(NAME) TEST_CASE_REFLECTION_LOADER(NAME, TestTypeLoader_Comparison)

TEST_FILE
{
	TEST_CASE_REFLECTION(TestCompareInt<vint8_t>)
	TEST_CASE_REFLECTION(TestCompareInt<vint16_t>)
	TEST_CASE_REFLECTION(TestCompareInt<vint32_t>)
	TEST_CASE_REFLECTION(TestCompareInt<vint64_t>)
	TEST_CASE_REFLECTION(TestCompareInt<vuint8_t>)
	TEST_CASE_REFLECTION(TestCompareInt<vuint16_t>)
	TEST_CASE_REFLECTION(TestCompareInt<vuint32_t>)
	TEST_CASE_REFLECTION(TestCompareInt<vuint64_t>)
	TEST_CASE_REFLECTION(TestCompareInt<wchar_t>)
	TEST_CASE_REFLECTION(TestCompareFloat<float>)
	TEST_CASE_REFLECTION(TestCompareFloat<double>)
	TEST_CASE_REFLECTION(TestCompareBool)
	TEST_CASE_REFLECTION(TestCompareString)
	TEST_CASE_REFLECTION(TestCompareLocale)
	TEST_CASE_REFLECTION(TestCompareStruct)
}
