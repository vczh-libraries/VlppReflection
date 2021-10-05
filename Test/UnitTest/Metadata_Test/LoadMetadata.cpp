#include "../../../Source/Reflection/GuiTypeDescriptorReflection.h"

using namespace vl;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::reflection;
using namespace vl::reflection::description;

extern WString GetTestOutputPath();

BEGIN_GLOBAL_STORAGE_CLASS(MetaonlyTypeDescriptors)
	Ptr<ITypeLoader>		typeLoader;

INITIALIZE_GLOBAL_STORAGE_CLASS
collections::Dictionary<WString, Ptr<ISerializableType>> serializableTypes;
	FileStream fileStream(GetTestOutputPath() + L"Reflection.bin", FileStream::ReadOnly);
	typeLoader = LoadMetaonlyTypes(fileStream, serializableTypes);

FINALIZE_GLOBAL_STORAGE_CLASS

END_GLOBAL_STORAGE_CLASS(MetaonlyTypeDescriptors)

TEST_FILE
{
	TEST_CASE(L"Run LoadMetaonlyTypes()")
	{
		GetGlobalTypeManager()->AddTypeLoader(GetMetaonlyTypeDescriptors().typeLoader);
		GetGlobalTypeManager()->Load();
		{
			FileStream fileStream(GetTestOutputPath() + L"Reflection2.txt", FileStream::WriteOnly);
			BomEncoder encoder(BomEncoder::Utf16);
			EncoderStream encoderStream(fileStream, encoder);
			StreamWriter writer(encoderStream);
			LogTypeManager(writer);
		}
		{
			auto first = File(GetTestOutputPath() + L"Reflection.txt").ReadAllTextByBom();
			auto second = File(GetTestOutputPath() + L"Reflection2.txt").ReadAllTextByBom();
			TEST_ASSERT(first == second);
		}
		TEST_ASSERT(ResetGlobalTypeManager());
	});
}

bool LoadPredefinedTypesForTestCase()
{
	return GetGlobalTypeManager()->AddTypeLoader(GetMetaonlyTypeDescriptors().typeLoader);
}