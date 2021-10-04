#include "../../../Source/Reflection/GuiTypeDescriptorReflection.h"

using namespace vl;
using namespace vl::stream;
using namespace vl::reflection;
using namespace vl::reflection::description;

extern WString GetTestOutputPath();

bool LoadPredefinedTypesForTestCase()
{
	TEST_ASSERT(LoadPredefinedTypes());
	GetGlobalTypeManager()->Load();
	static bool generated = false;
	if (!generated)
	{
		generated = true;
		FileStream fileStream(GetTestOutputPath() + L"Reflection.bin", FileStream::WriteOnly);
		GenerateMetaonlyTypes(fileStream);
	}
	return true;
}