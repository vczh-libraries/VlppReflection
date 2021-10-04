#include "../../../Source/Reflection/GuiTypeDescriptorReflection.h"

using namespace vl;
using namespace vl::stream;
using namespace vl::reflection;
using namespace vl::reflection::description;

extern WString GetTestOutputPath();

TEST_FILE
{
	TEST_CASE(L"Run GenerateMetaonlyTypes()")
	{
		TEST_ASSERT(LoadPredefinedTypes());
		GetGlobalTypeManager()->Load();
		{
			FileStream fileStream(GetTestOutputPath() + L"Reflection.bin", FileStream::WriteOnly);
			GenerateMetaonlyTypes(fileStream);
		}
		{
			FileStream fileStream(GetTestOutputPath() + L"Reflection.txt", FileStream::WriteOnly);
			BomEncoder encoder(BomEncoder::Utf16);
			EncoderStream encoderStream(fileStream, encoder);
			StreamWriter writer(encoderStream);
			LogTypeManager(writer);
		}
		TEST_ASSERT(ResetGlobalTypeManager());
	});
}

bool LoadPredefinedTypesForTestCase()
{
	return LoadPredefinedTypes();
}