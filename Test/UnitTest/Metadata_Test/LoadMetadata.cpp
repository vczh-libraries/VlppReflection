#include "../../../Source/Reflection/GuiTypeDescriptorReflection.h"

using namespace vl;
using namespace vl::reflection;
using namespace vl::reflection::description;

BEGIN_GLOBAL_STORAGE_CLASS(MetaonlyTypeDescriptors)

INITIALIZE_GLOBAL_STORAGE_CLASS

FINALIZE_GLOBAL_STORAGE_CLASS

END_GLOBAL_STORAGE_CLASS(MetaonlyTypeDescriptors)

bool LoadPredefinedTypesForTestCase()
{
	static bool testedMeta = false;
	if (!testedMeta)
	{
		testedMeta = true;
		// TODO: ensure loaded type descriptors contain exact the same information
	}
	return false;
}