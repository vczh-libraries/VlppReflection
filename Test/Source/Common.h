#ifndef VCZH_REFLECTION_UNITTEST_COMMON
#define VCZH_REFLECTION_UNITTEST_COMMON

#include "../../Source/Reflection/Reflection/Reflection.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection;
using namespace vl::reflection::description;

#define TEST_CASE_REFLECTION_BODY(NAME)\
		try\
		{\
			TEST_ASSERT(GetGlobalTypeManager()->Load());\
			{\
				NAME();\
			}\
		}\
		catch (...)\
		{\
			TEST_ASSERT(ResetGlobalTypeManager());\
			throw;\
		}\
		TEST_ASSERT(ResetGlobalTypeManager());\

#define TEST_CASE_REFLECTION_NOLOADER(NAME)\
	TEST_CASE(L ## #NAME)\
	{\
		TEST_ASSERT(LoadPredefinedTypes());\
		TEST_CASE_REFLECTION_BODY(NAME)\
	});\

#define TEST_CASE_REFLECTION_LOADER(NAME, LOADER)\
	TEST_CASE(L ## #NAME)\
	{\
		TEST_ASSERT(LoadPredefinedTypes());\
		TEST_ASSERT(GetGlobalTypeManager()->AddTypeLoader(new LOADER));\
		TEST_CASE_REFLECTION_BODY(NAME)\
	});\

#endif