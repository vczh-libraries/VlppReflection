#include "../../../Source/Reflection/Reflection/Reflection.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::filesystem;
using namespace vl::stream;
using namespace vl::reflection;
using namespace vl::reflection::description;

extern WString GetTestOutputPath();

#ifdef VCZH_64
#define REFLECTION_BIN L"Reflection64.bin"
#define REFLECTION_OUTPUT L"Reflection64.txt"
#define REFLECTION_BASELINE L"Reflection64.txt"
#else
#define REFLECTION_BIN L"Reflection32.bin"
#define REFLECTION_OUTPUT L"Reflection32.txt"
#define REFLECTION_BASELINE L"Reflection32.txt"
#endif

bool LoadPredefinedTypesForTestCase()
{
	auto result = LoadPredefinedTypes();
	GetGlobalTypeManager()->Load();
	return result;
}

TEST_FILE
{
	TEST_CASE(L"Run GenerateMetaonlyTypes()")
	{
		TEST_ASSERT(LoadPredefinedTypesForTestCase());
		{
			FileStream fileStream(GetTestOutputPath() + REFLECTION_BIN, FileStream::WriteOnly);
			GenerateMetaonlyTypes(fileStream);
		}
		{
			FileStream fileStream(GetTestOutputPath() + REFLECTION_OUTPUT, FileStream::WriteOnly);
			BomEncoder encoder(BomEncoder::Utf8);
			EncoderStream encoderStream(fileStream, encoder);
			StreamWriter writer(encoderStream);
			LogTypeManager(writer);
		}
		{
			List<WString> first, second;
			File(GetTestOutputPath() + REFLECTION_OUTPUT).ReadAllLinesByBom(first);
			File(GetTestOutputPath() + L"../Resources/Baseline/" REFLECTION_BASELINE).ReadAllLinesByBom(second);
			TEST_ASSERT(CompareEnumerable(first, second) == 0);
		}
		TEST_ASSERT(ResetGlobalTypeManager());
	});
}