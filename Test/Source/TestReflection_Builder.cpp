#include "../../Source/Reflection/GuiTypeDescriptorReflection.h"

using namespace vl;
using namespace vl::reflection;
using namespace vl::reflection::description;
using namespace vl::stream;

extern WString GetTestOutputPath();

namespace reflection_test_builder
{
	void TestReflectionBuilder()
	{
		FileStream fileStream(GetTestOutputPath() + L"ReflectionWithTestTypes.txt", FileStream::WriteOnly);
		BomEncoder encoder(BomEncoder::Utf16);
		EncoderStream encoderStream(fileStream, encoder);
		StreamWriter writer(encoderStream);
		LogTypeManager(writer);

		TEST_ASSERT(GetTypeDescriptor<Value>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Object);
		TEST_ASSERT(GetTypeDescriptor<vuint8_t>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<vuint16_t>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<vuint32_t>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<vuint64_t>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<vint8_t>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<vint16_t>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<vint32_t>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<vint64_t>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<float>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<double>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<wchar_t>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<WString>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<Locale>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<bool>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<DateTime>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Struct);
		TEST_ASSERT(GetTypeDescriptor<VoidValue>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Primitive);
		TEST_ASSERT(GetTypeDescriptor<IDescriptable>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::IDescriptable);
		TEST_ASSERT(GetTypeDescriptor<DescriptableObject>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Class);
		TEST_ASSERT(GetTypeDescriptor<IValueEnumerator>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IValueEnumerable>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IValueReadonlyList>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IValueList>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IValueObservableList>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IValueReadonlyDictionary>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IValueDictionary>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IValueInterfaceProxy>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IValueFunctionProxy>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IValueSubscription>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IValueCallStack>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IValueException>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IValueType>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IEnumType>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<ISerializableType>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<ITypeInfo>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<ITypeInfo::Decorator>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::NormalEnum);
		TEST_ASSERT(GetTypeDescriptor<IMemberInfo>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IEventHandler>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IEventInfo>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IPropertyInfo>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IParameterInfo>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IMethodInfo>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<IMethodGroupInfo>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
		TEST_ASSERT(GetTypeDescriptor<ITypeDescriptor>()->GetTypeDescriptorFlags() == TypeDescriptorFlags::Interface);
	}
}
using namespace reflection_test_builder;

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
	TEST_CASE_REFLECTION(TestReflectionBuilder)
}
