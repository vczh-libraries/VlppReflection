#include "TestReflection_Attribute.h"

namespace TestReflection_TestObjects_Attribute
{
	struct MyAttribute
	{
		WString								name;
		vint								number;

		auto operator<=>(const MyAttribute&) const = default;
	};

	struct EmptyAttribute
	{
		auto operator<=>(const EmptyAttribute&) const = default;
	};

	struct AttributeRecord
	{
		vint								x = 0;
		vint								y = 0;

		auto operator<=>(const AttributeRecord&) const = default;
	};

	class AttributeTarget : public Description<AttributeTarget>
	{
	protected:
		vint								value = 0;
	public:
		vint								fieldValue = 0;
		Event<void(vint)>					Changed;

		AttributeTarget() = default;

		AttributeTarget(vint seed)
			:value(seed)
			, fieldValue(seed + 1)
		{
		}

		vint GetValue()
		{
			return value;
		}

		void SetValue(vint newValue)
		{
			value = newValue;
			Changed(newValue);
		}

		vint Sum(vint x, vint y)
		{
			return value + x + y;
		}
	};
}
using namespace TestReflection_TestObjects_Attribute;

#define _ ,

#define TYPE_LIST(F)\
	F(MyAttribute)\
	F(EmptyAttribute)\
	F(AttributeRecord)\
	F(AttributeTarget)\

BEGIN_TYPE_INFO_NAMESPACE

	TYPE_LIST(DECL_TYPE_INFO)
	TYPE_LIST(IMPL_CPP_TYPE_INFO)

#if !defined(VCZH_DEBUG_NO_REFLECTION) && !defined(VCZH_DEBUG_METAONLY_REFLECTION)

	BEGIN_STRUCT_MEMBER(MyAttribute)
		STRUCT_MEMBER(name)
		STRUCT_MEMBER(number)
	END_STRUCT_MEMBER(MyAttribute)

	BEGIN_STRUCT_MEMBER(EmptyAttribute)
	END_STRUCT_MEMBER(EmptyAttribute)

	BEGIN_STRUCT_MEMBER(AttributeRecord)
		STRUCT_MEMBER(x)
		ATTRIBUTE_MEMBER(MyAttribute, L"struct-field", vint(91))
		STRUCT_MEMBER(y)
		ATTRIBUTE_MEMBER(EmptyAttribute)
	END_STRUCT_MEMBER(AttributeRecord)

	BEGIN_CLASS_MEMBER(AttributeTarget)
		ATTRIBUTE_TYPE(MyAttribute, L"type", vint(1))
		ATTRIBUTE_TYPE(EmptyAttribute)

		CLASS_MEMBER_CONSTRUCTOR(Ptr<AttributeTarget>(), NO_PARAMETER)
		ATTRIBUTE_MEMBER(MyAttribute, L"default-ctor", vint(2))

		CLASS_MEMBER_CONSTRUCTOR(Ptr<AttributeTarget>(vint), {L"seed"})
		ATTRIBUTE_MEMBER(MyAttribute, L"seed-ctor", vint(3))
		ATTRIBUTE_PARAMETER(L"seed", MyAttribute, L"ctor-param", vint(4))

		CLASS_MEMBER_EVENT(Changed)
		ATTRIBUTE_MEMBER(MyAttribute, L"event", vint(5))

		CLASS_MEMBER_FIELD(fieldValue)
		ATTRIBUTE_MEMBER(MyAttribute, L"field", vint(6))
		ATTRIBUTE_MEMBER(EmptyAttribute)

		CLASS_MEMBER_PROPERTY_FAST(Value)
		ATTRIBUTE_MEMBER(MyAttribute, L"property", vint(7))

		CLASS_MEMBER_METHOD(Sum, {L"x" _ L"y"})
		ATTRIBUTE_MEMBER(MyAttribute, L"method", vint(8))
		ATTRIBUTE_PARAMETER(L"x", MyAttribute, L"param-x", vint(9))
		ATTRIBUTE_PARAMETER(L"y", EmptyAttribute)
	END_CLASS_MEMBER(AttributeTarget)

	class TestTypeLoader_Attribute : public Object, public ITypeLoader
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

#endif

END_TYPE_INFO_NAMESPACE

#undef TYPE_LIST
#undef _

namespace reflection_test_attribute
{
	namespace
	{
		vint64_t ReadAttributeInteger(const Value& value)
		{
			auto td = value.GetTypeDescriptor();
			TEST_ASSERT(td != nullptr);
			if (td == GetTypeDescriptor<vint32_t>())
			{
				return UnboxValue<vint32_t>(value);
			}
			if (td == GetTypeDescriptor<vint64_t>())
			{
				return UnboxValue<vint64_t>(value);
			}
			TEST_ASSERT(false);
			return 0;
		}

		void AssertEmptyAttribute(IAttributeInfo* info)
		{
			TEST_ASSERT(info != nullptr);
			TEST_ASSERT(info->GetAttributeType()->GetTypeName() == TypeInfo<EmptyAttribute>::content.typeName);
			TEST_ASSERT(info->GetAttributeValueCount() == 0);
		}

		void AssertMyAttribute(IAttributeInfo* info, const WString& name, vint number)
		{
			TEST_ASSERT(info != nullptr);
			TEST_ASSERT(info->GetAttributeType()->GetTypeName() == TypeInfo<MyAttribute>::content.typeName);
			TEST_ASSERT(info->GetAttributeValueCount() == 2);
			TEST_ASSERT(UnboxValue<WString>(info->GetAttributeValue(0)) == name);
			TEST_ASSERT(ReadAttributeInteger(info->GetAttributeValue(1)) == number);
		}
	}

	Ptr<ITypeLoader> CreateTestTypeLoader_Attribute()
	{
#if !defined(VCZH_DEBUG_NO_REFLECTION) && !defined(VCZH_DEBUG_METAONLY_REFLECTION)
		return Ptr(new TestTypeLoader_Attribute);
#else
		return nullptr;
#endif
	}

	void TestReflectionAttributes()
	{
		auto td = GetTypeDescriptor(TypeInfo<AttributeTarget>::content.typeName);
		TEST_ASSERT(td != nullptr);
		TEST_ASSERT(td->GetAttributeCount() == 2);
		AssertMyAttribute(td->GetAttribute(0), L"type", 1);
		AssertEmptyAttribute(td->GetAttribute(1));

		auto eventInfo = td->GetEventByName(L"Changed", false);
		TEST_ASSERT(eventInfo != nullptr);
		TEST_ASSERT(eventInfo->GetAttributeCount() == 1);
		AssertMyAttribute(eventInfo->GetAttribute(0), L"event", 5);

		auto fieldInfo = td->GetPropertyByName(L"fieldValue", false);
		TEST_ASSERT(fieldInfo != nullptr);
		TEST_ASSERT(fieldInfo->GetAttributeCount() == 2);
		AssertMyAttribute(fieldInfo->GetAttribute(0), L"field", 6);
		AssertEmptyAttribute(fieldInfo->GetAttribute(1));

		auto propertyInfo = td->GetPropertyByName(L"Value", false);
		TEST_ASSERT(propertyInfo != nullptr);
		TEST_ASSERT(propertyInfo->GetAttributeCount() == 1);
		AssertMyAttribute(propertyInfo->GetAttribute(0), L"property", 7);

		auto methodGroup = td->GetMethodGroupByName(L"Sum", false);
		TEST_ASSERT(methodGroup != nullptr);
		TEST_ASSERT(methodGroup->GetMethodCount() == 1);
		auto methodInfo = methodGroup->GetMethod(0);
		TEST_ASSERT(methodInfo->GetAttributeCount() == 1);
		AssertMyAttribute(methodInfo->GetAttribute(0), L"method", 8);
		TEST_ASSERT(methodInfo->GetParameterCount() == 2);
		AssertMyAttribute(methodInfo->GetParameter(0)->GetAttribute(0), L"param-x", 9);
		AssertEmptyAttribute(methodInfo->GetParameter(1)->GetAttribute(0));

		auto constructorGroup = td->GetConstructorGroup();
		TEST_ASSERT(constructorGroup != nullptr);
		TEST_ASSERT(constructorGroup->GetMethodCount() == 2);
		AssertMyAttribute(constructorGroup->GetMethod(0)->GetAttribute(0), L"default-ctor", 2);
		auto seededCtor = constructorGroup->GetMethod(1);
		AssertMyAttribute(seededCtor->GetAttribute(0), L"seed-ctor", 3);
		TEST_ASSERT(seededCtor->GetParameterCount() == 1);
		AssertMyAttribute(seededCtor->GetParameter(0)->GetAttribute(0), L"ctor-param", 4);

		auto structTd = GetTypeDescriptor(TypeInfo<AttributeRecord>::content.typeName);
		TEST_ASSERT(structTd != nullptr);
		auto xField = structTd->GetPropertyByName(L"x", false);
		TEST_ASSERT(xField != nullptr);
		TEST_ASSERT(xField->GetAttributeCount() == 1);
		AssertMyAttribute(xField->GetAttribute(0), L"struct-field", 91);

		auto yField = structTd->GetPropertyByName(L"y", false);
		TEST_ASSERT(yField != nullptr);
		TEST_ASSERT(yField->GetAttributeCount() == 1);
		AssertEmptyAttribute(yField->GetAttribute(0));
	}
}
