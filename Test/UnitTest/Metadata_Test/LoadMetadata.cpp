#include "../../../Source/Reflection/Reflection/Reflection.h"
#include "../../Source/TestReflection_Attribute.h"

using namespace vl;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::reflection;
using namespace vl::reflection::description;
using namespace reflection_test_attribute;

extern WString GetTestMetadataPath();

#ifdef VCZH_64
#define REFLECTION_BIN L"Reflection64.bin"
#define REFLECTION_OUTPUT L"Reflection64[2].txt"
#define REFLECTION_BASELINE L"Reflection64.txt"
#define REFLECTION_ATTRIBUTE_BIN L"ReflectionAttribute64.bin"
#define REFLECTION_ATTRIBUTE_OUTPUT L"ReflectionAttribute64[2].txt"
#define REFLECTION_ATTRIBUTE_BASELINE L"ReflectionAttribute64.txt"
#else
#define REFLECTION_BIN L"Reflection32.bin"
#define REFLECTION_OUTPUT L"Reflection32[2].txt"
#define REFLECTION_BASELINE L"Reflection32.txt"
#define REFLECTION_ATTRIBUTE_BIN L"ReflectionAttribute32.bin"
#define REFLECTION_ATTRIBUTE_OUTPUT L"ReflectionAttribute32[2].txt"
#define REFLECTION_ATTRIBUTE_BASELINE L"ReflectionAttribute32.txt"
#endif

#define INSTALL_SERIALIZABLE_TYPE(TYPE)\
	serializableTypes.Add(TypeInfo<TYPE>::content.typeName, Ptr(new SerializableType<TYPE>()));

BEGIN_GLOBAL_STORAGE_CLASS(MetaonlyTypeDescriptors)
	collections::Dictionary<WString, Ptr<ISerializableType>>	serializableTypes;
	Ptr<ITypeLoader>											baseTypeLoader;

INITIALIZE_GLOBAL_STORAGE_CLASS
	REFLECTION_PREDEFINED_SERIALIZABLE_TYPES(INSTALL_SERIALIZABLE_TYPE)
	FileStream fileStream(GetTestMetadataPath() + REFLECTION_BIN, FileStream::ReadOnly);
	baseTypeLoader = LoadMetaonlyTypes(fileStream, serializableTypes);

FINALIZE_GLOBAL_STORAGE_CLASS
	baseTypeLoader = nullptr;
	serializableTypes.Clear();

END_GLOBAL_STORAGE_CLASS(MetaonlyTypeDescriptors)

bool LoadPredefinedTypesForTestCase()
{
	{
		TEST_ASSERT(GetTypeDescriptor(TypeInfo<DateTime>::content.typeName) == nullptr);
		TEST_ASSERT(GetTypeDescriptor<DateTime>() == nullptr);
		TEST_ASSERT(GetTypeDescriptor<DateTime>() == nullptr);
	}

	auto tm = GetGlobalTypeManager();
	auto result = tm->AddTypeLoader(GetMetaonlyTypeDescriptors().baseTypeLoader);
	tm->Load();
	{
		// Ensure that the type version is changed
		static vint previousTypeVersion = -1;
		TEST_ASSERT(previousTypeVersion != tm->GetTypeVersion());
		previousTypeVersion = tm->GetTypeVersion();
	}
	{
		// Ensure all ITypeDescriptor* cache is updated
		auto td = GetTypeDescriptor(TypeInfo<DateTime>::content.typeName);
		TEST_ASSERT(td != nullptr);
		TEST_ASSERT(GetTypeDescriptor<DateTime>() == td);
		TEST_ASSERT(GetTypeDescriptor<DateTime>() == td);
	}
	return result;
}

TEST_FILE
{
	TEST_CASE(L"Run LoadMetaonlyTypes()")
	{
		auto&& descriptors = GetMetaonlyTypeDescriptors();
		{
			FileStream fileStream(GetTestMetadataPath() + REFLECTION_ATTRIBUTE_BIN, FileStream::ReadOnly);
			TEST_ERROR(LoadMetaonlyTypes(fileStream, descriptors.serializableTypes));
		}
		TEST_ASSERT(ResetGlobalTypeManager());

		TEST_ASSERT(LoadPredefinedTypesForTestCase());
		{
			FileStream fileStream(GetTestMetadataPath() + REFLECTION_OUTPUT, FileStream::WriteOnly);
			BomEncoder encoder(BomEncoder::Utf8);
			EncoderStream encoderStream(fileStream, encoder);
			StreamWriter writer(encoderStream);
			LogTypeManager(writer);
		}
		{
			auto first = File(GetTestMetadataPath() + REFLECTION_BASELINE).ReadAllTextByBom();
			auto second = File(GetTestMetadataPath() + REFLECTION_OUTPUT).ReadAllTextByBom();
			TEST_ASSERT(first == second);
		}

		auto tm = GetGlobalTypeManager();
		Ptr<ITypeLoader> attributeTypeLoader;
		{
			FileStream fileStream(GetTestMetadataPath() + REFLECTION_ATTRIBUTE_BIN, FileStream::ReadOnly);
			attributeTypeLoader = LoadMetaonlyTypes(fileStream, descriptors.serializableTypes);
		}
		TEST_ASSERT(tm->AddTypeLoader(attributeTypeLoader));
		TestReflectionAttributes();
		{
			FileStream fileStream(GetTestMetadataPath() + REFLECTION_ATTRIBUTE_OUTPUT, FileStream::WriteOnly);
			BomEncoder encoder(BomEncoder::Utf8);
			EncoderStream encoderStream(fileStream, encoder);
			StreamWriter writer(encoderStream);
			LogTypeManager(writer);
		}
		{
			auto first = File(GetTestMetadataPath() + REFLECTION_ATTRIBUTE_BASELINE).ReadAllTextByBom();
			auto second = File(GetTestMetadataPath() + REFLECTION_ATTRIBUTE_OUTPUT).ReadAllTextByBom();
			TEST_ASSERT(first == second);
		}
		TEST_ASSERT(ResetGlobalTypeManager());
	});
}
