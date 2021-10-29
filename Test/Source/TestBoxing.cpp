#include "../../Source/Reflection/GuiTypeDescriptorReflection.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection;
using namespace vl::reflection::description;

namespace TestBoxing_TestObjects_Boxing
{
	class Base : public Object, public Description<Base>
	{
	};

	class Derived : public Base, public Description<Derived>
	{
	};
}
using namespace TestBoxing_TestObjects_Boxing;

#ifndef VCZH_DEBUG_NO_REFLECTION

#define _ ,

#define TYPE_LIST(F)\
	F(Base)\
	F(Derived)\

BEGIN_TYPE_INFO_NAMESPACE

	TYPE_LIST(DECL_TYPE_INFO)
	TYPE_LIST(IMPL_CPP_TYPE_INFO)

END_TYPE_INFO_NAMESPACE

#undef TYPE_LIST

#endif

TEST_FILE
{
	TEST_CASE(L"Test DescriptableObject: ReferenceCounterOperator")
	{
		TEST_ASSERT((!std::is_convertible_v<vint*, DescriptableObject*>));
		TEST_ASSERT((std::is_convertible_v<DescriptableObject*, DescriptableObject*>));
		TEST_ASSERT((std::is_convertible_v<IDescriptable*, DescriptableObject*>));
		TEST_ASSERT((std::is_convertible_v<Base*, DescriptableObject*>));
		TEST_ASSERT((std::is_convertible_v<Derived*, DescriptableObject*>));

		Base* raw = new Base;
		volatile vint* counter = ReferenceCounterOperator<Base>::CreateCounter(raw);
		TEST_ASSERT(0 == *counter);
		{
			Ptr<Base> ptr1 = raw;
			TEST_ASSERT(1 == *counter);
			{
				Ptr<Base> ptr2 = raw;
				TEST_ASSERT(2 == *counter);
				{
					Ptr<Base> ptr3 = raw;
					TEST_ASSERT(3 == *counter);
				}
				TEST_ASSERT(2 == *counter);
			}
			TEST_ASSERT(1 == *counter);
		}
	});

#ifndef VCZH_DEBUG_NO_REFLECTION

	TEST_CASE(L"Test DescriptableObject: Boxing and unboxing")
	{
		{
			auto value = BoxValue<vint>(0);
			TEST_ASSERT(UnboxValue<vint>(value) == 0);
		}
		{
			auto value = BoxValue<WString>(L"abc");
			TEST_ASSERT(UnboxValue<WString>(value) == L"abc");
		}
		{
			Ptr<Base> base = MakePtr<Base>();
			{
				auto value = BoxValue<Base*>(base.Obj());
				TEST_ASSERT(UnboxValue<Base*>(value) == base.Obj());
				TEST_ASSERT(UnboxValue<Ptr<Base>>(value) == base);
			}
			{
				auto value = BoxValue<Ptr<Base>>(base);
				TEST_ASSERT(UnboxValue<Base*>(value) == base.Obj());
				TEST_ASSERT(UnboxValue<Ptr<Base>>(value) == base);
			}
		}
		{
			List<vint> numbers;
			numbers.Add(1);
			numbers.Add(2);
			numbers.Add(3);

			auto value = BoxParameter(numbers);
			auto numbers2 = UnboxParameter<List<vint>>(value);

			TEST_ASSERT(numbers2.Ref().Count() == 3);
			TEST_ASSERT(numbers2.Ref()[0] == 1);
			TEST_ASSERT(numbers2.Ref()[1] == 2);
			TEST_ASSERT(numbers2.Ref()[2] == 3);
		}
	});

#endif
}
