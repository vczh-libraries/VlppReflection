#include "../../../Source/Reflection/GuiTypeDescriptorReflection.h"

using namespace vl;
using namespace vl::reflection;
using namespace vl::reflection::description;

bool LoadPredefinedTypesForTestCase()
{
	TEST_ASSERT(LoadPredefinedTypes());
	static bool generated = false;
	if (!generated)
	{
		generated = true;
		// TODO: write type descriptors to files
	}
	return true;
}