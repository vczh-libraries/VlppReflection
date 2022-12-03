#include "Common.h"

using namespace vl;
using namespace vl::reflection;
using namespace vl::reflection::description;

namespace TestReflection_TestObjects_Event
{
	class EventRaiser : public Description<EventRaiser>
	{
	protected:
		vint									value;
	public:
		Event<void(vint, vint)>					ValueChanged;

		EventRaiser()
			:value(0)
		{
		}

		vint GetValue()
		{
			return value;
		}

		void SetValue(vint newValue)
		{
			vint oldValue = value;
			value = newValue;
			ValueChanged(oldValue, value);
		}
	};
}
using namespace TestReflection_TestObjects_Event;

#define _ ,

#define TYPE_LIST(F)\
	F(EventRaiser)\

BEGIN_TYPE_INFO_NAMESPACE

	TYPE_LIST(DECL_TYPE_INFO)
	TYPE_LIST(IMPL_CPP_TYPE_INFO)

	BEGIN_CLASS_MEMBER(EventRaiser)
		CLASS_MEMBER_CONSTRUCTOR(Ptr<EventRaiser>(), NO_PARAMETER)

		CLASS_MEMBER_EVENT(ValueChanged)
		CLASS_MEMBER_PROPERTY_EVENT_FAST(Value, ValueChanged)
	END_CLASS_MEMBER(EventRaiser)

	class TestTypeLoader_Event : public Object, public ITypeLoader
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

namespace reflection_test_event
{
	void TestReflectionEvent()
	{
		Value eventRaiser = Value::Create(L"EventRaiser");
		vint oldValue = 0;
		vint newValue = 0;
		auto eventHandler = eventRaiser.AttachEvent(
			L"ValueChanged",
			BoxParameter(Func([&](vint _oldValue, vint _newValue)
			{
				oldValue = _oldValue;
				newValue = _newValue;
			})));
		TEST_ASSERT(eventHandler->IsAttached() == true);

		TEST_ASSERT(UnboxValue<vint>(eventRaiser.GetProperty(L"Value")) == 0);
		TEST_ASSERT(oldValue == 0);
		TEST_ASSERT(newValue == 0);

		eventRaiser.SetProperty(L"Value", BoxValue<vint>(100));
		TEST_ASSERT(UnboxValue<vint>(eventRaiser.GetProperty(L"Value")) == 100);
		TEST_ASSERT(oldValue == 0);
		TEST_ASSERT(newValue == 100);

		eventRaiser.SetProperty(L"Value", BoxValue<vint>(200));
		TEST_ASSERT(UnboxValue<vint>(eventRaiser.GetProperty(L"Value")) == 200);
		TEST_ASSERT(oldValue == 100);
		TEST_ASSERT(newValue == 200);

		TEST_ASSERT(eventRaiser.DetachEvent(L"ValueChanged", eventHandler) == true);
		TEST_ASSERT(eventHandler->IsAttached() == false);

		eventRaiser.SetProperty(L"Value", BoxValue<vint>(300));
		TEST_ASSERT(UnboxValue<vint>(eventRaiser.GetProperty(L"Value")) == 300);
		TEST_ASSERT(oldValue == 100);
		TEST_ASSERT(newValue == 200);

		TEST_ASSERT(eventRaiser.DetachEvent(L"ValueChanged", eventHandler) == false);
	}
}
using namespace reflection_test_event;

#define TEST_CASE_REFLECTION(NAME) TEST_CASE_REFLECTION_LOADER(NAME, TestTypeLoader_Event)

TEST_FILE
{
	TEST_CASE_REFLECTION(TestReflectionEvent)
}
