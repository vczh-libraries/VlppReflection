#include "Common.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection;
using namespace vl::reflection::description;

namespace TestReflection_TestObjects
{
	struct ValueToCompare
	{
		vint a;
		WString b;

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
	void TestCompareInt()
	{
	}

	template<typename T>
	void TestCompareFloat()
	{
	}

	void TestCompareBool()
	{
	}

	void TestCompareString()
	{
	}

	void TestCompareLocale()
	{
	}

	void TestCompareStruct()
	{
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
	TEST_CASE_REFLECTION(TestCompareFloat<float>)
	TEST_CASE_REFLECTION(TestCompareFloat<double>)
	TEST_CASE_REFLECTION(TestCompareBool)
	TEST_CASE_REFLECTION(TestCompareString)
	TEST_CASE_REFLECTION(TestCompareLocale)
	TEST_CASE_REFLECTION(TestCompareStruct)
}
