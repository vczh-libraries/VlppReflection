#include "../../../Source/Reflection/GuiTypeDescriptorReflection.h"

using namespace vl;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::reflection;
using namespace vl::reflection::description;

extern WString GetTestOutputPath();

#ifdef VCZH_64
#define REFLECTION_BIN L"Reflection64.bin"
#define REFLECTION_OUTPUT L"Reflection64[2].txt"
#define REFLECTION_BASELINE L"Reflection64.txt"
#else
#define REFLECTION_BIN L"Reflection32.bin"
#define REFLECTION_OUTPUT L"Reflection32[2].txt"
#define REFLECTION_BASELINE L"Reflection32.txt"
#endif

#define INSTALL_SERIALIZABLE_TYPE(TYPE)\
	serializableTypes.Add(TypeInfo<TYPE>::content.typeName, MakePtr<SerializableType<TYPE>>());

BEGIN_GLOBAL_STORAGE_CLASS(MetaonlyTypeDescriptors)
	Ptr<ITypeLoader>		typeLoader;

INITIALIZE_GLOBAL_STORAGE_CLASS
	collections::Dictionary<WString, Ptr<ISerializableType>> serializableTypes;
	REFLECTION_PREDEFINED_SERIALIZABLE_TYPES(INSTALL_SERIALIZABLE_TYPE)
	FileStream fileStream(GetTestOutputPath() + REFLECTION_BIN, FileStream::ReadOnly);
	typeLoader = LoadMetaonlyTypes(fileStream, serializableTypes);

FINALIZE_GLOBAL_STORAGE_CLASS
	typeLoader = nullptr;

END_GLOBAL_STORAGE_CLASS(MetaonlyTypeDescriptors)

TEST_FILE
{
	TEST_CASE(L"Run LoadMetaonlyTypes()")
	{
		GetGlobalTypeManager()->AddTypeLoader(GetMetaonlyTypeDescriptors().typeLoader);
		GetGlobalTypeManager()->Load();
		{
			FileStream fileStream(GetTestOutputPath() + REFLECTION_OUTPUT, FileStream::WriteOnly);
			BomEncoder encoder(BomEncoder::Utf16);
			EncoderStream encoderStream(fileStream, encoder);
			StreamWriter writer(encoderStream);
			LogTypeManager(writer);
		}
		{
			auto first = File(GetTestOutputPath() + REFLECTION_BASELINE).ReadAllTextByBom();
			auto second = File(GetTestOutputPath() + REFLECTION_OUTPUT).ReadAllTextByBom();
			TEST_ASSERT(first == second);
		}
		TEST_ASSERT(ResetGlobalTypeManager());
	});
}

bool LoadPredefinedTypesForTestCase()
{
	return GetGlobalTypeManager()->AddTypeLoader(GetMetaonlyTypeDescriptors().typeLoader);
}