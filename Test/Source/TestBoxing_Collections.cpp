#include "../../Source/Reflection/GuiTypeDescriptorReflection.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection;
using namespace vl::reflection::description;

namespace reflection_test_boxunboxcollections
{
	void TestLazyList()
	{
	}
}
using namespace reflection_test_boxunboxcollections;

#define TEST_CASE_REFLECTION(NAME)\
	TEST_CASE(L ## #NAME)\
	{\
		TEST_ASSERT(LoadPredefinedTypes());\
		TEST_ASSERT(GetGlobalTypeManager()->Load());\
		{\
			NAME();\
		}\
		TEST_ASSERT(ResetGlobalTypeManager());\
	});\

TEST_FILE
{
	TEST_CASE_REFLECTION(TestLazyList)
}
