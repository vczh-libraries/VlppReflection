#include "../../../Source/Reflection/Reflection/Reflection.h"
#include "../../Source/TestReflection_Attribute.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::filesystem;
using namespace vl::stream;
using namespace vl::reflection;
using namespace vl::reflection::description;
using namespace reflection_test_attribute;

extern WString GetTestMetadataPath();

#ifdef VCZH_64
#define REFLECTION_BIN L"Reflection64.bin"
#define REFLECTION_OUTPUT L"Reflection64.txt"
#define REFLECTION_ATTRIBUTE_BIN L"ReflectionAttribute64.bin"
#define REFLECTION_ATTRIBUTE_OUTPUT L"ReflectionAttribute64.txt"
#else
#define REFLECTION_BIN L"Reflection32.bin"
#define REFLECTION_OUTPUT L"Reflection32.txt"
#define REFLECTION_ATTRIBUTE_BIN L"ReflectionAttribute32.bin"
#define REFLECTION_ATTRIBUTE_OUTPUT L"ReflectionAttribute32.txt"
#endif

TEST_FILE
{
	TEST_CASE(L"Run GenerateMetaonlyTypes()")
	{
		List<ITypeDescriptor*> emptyTypes;
		TEST_ASSERT(LoadPredefinedTypes());
		auto tm = GetGlobalTypeManager();
		TEST_ASSERT(tm->Load());
		{
			FileStream fileStream(GetTestMetadataPath() + REFLECTION_BIN, FileStream::WriteOnly);
			GenerateMetaonlyTypes(emptyTypes, fileStream);
		}
		{
			FileStream fileStream(GetTestMetadataPath() + REFLECTION_OUTPUT, FileStream::WriteOnly);
			BomEncoder encoder(BomEncoder::Utf8);
			EncoderStream encoderStream(fileStream, encoder);
			StreamWriter writer(encoderStream);
			LogTypeManager(writer);
		}

		List<ITypeDescriptor*> excludedTypes;
		excludedTypes.Add(nullptr);
		CollectRegisteredTypes(excludedTypes);
		TEST_ASSERT(excludedTypes.Count() == tm->GetTypeDescriptorCount());
		for (vint i = 0; i < excludedTypes.Count(); i++)
		{
			TEST_ASSERT(excludedTypes[i] == tm->GetTypeDescriptor(i));
		}
		for (vint i = 0; i < excludedTypes.Count() / 2; i++)
		{
			auto j = excludedTypes.Count() - 1 - i;
			auto td = excludedTypes[i];
			excludedTypes[i] = excludedTypes[j];
			excludedTypes[j] = td;
		}

		SortedList<WString> expectedForeignNames;
		for (vint i = 0; i < excludedTypes.Count(); i++)
		{
			TEST_ASSERT(!expectedForeignNames.Contains(excludedTypes[i]->GetTypeName()));
			expectedForeignNames.Add(excludedTypes[i]->GetTypeName());
		}

		TEST_ASSERT(tm->AddTypeLoader(CreateTestTypeLoader_Attribute()));
		TestReflectionAttributes();
		auto localTypeCount = tm->GetTypeDescriptorCount() - excludedTypes.Count();
		{
			FileStream fileStream(GetTestMetadataPath() + REFLECTION_ATTRIBUTE_BIN, FileStream::WriteOnly);
			GenerateMetaonlyTypes(excludedTypes, fileStream);
		}
		{
			FileStream fileStream(GetTestMetadataPath() + REFLECTION_ATTRIBUTE_OUTPUT, FileStream::WriteOnly);
			BomEncoder encoder(BomEncoder::Utf8);
			EncoderStream encoderStream(fileStream, encoder);
			StreamWriter writer(encoderStream);
			LogTypeManager(writer);
		}
		{
			FileStream fileStream(GetTestMetadataPath() + REFLECTION_BIN, FileStream::ReadOnly);
			stream::internal::ContextFreeReader reader(fileStream);
			List<WString> foreignNames;
			vint tdCount = 0;
			vint miCount = 0;
			vint piCount = 0;
			vint eiCount = 0;
			reader << foreignNames << tdCount << miCount << piCount << eiCount;
			TEST_ASSERT(foreignNames.Count() == 0);
			TEST_ASSERT(tdCount == excludedTypes.Count());
		}
		{
			FileStream fileStream(GetTestMetadataPath() + REFLECTION_ATTRIBUTE_BIN, FileStream::ReadOnly);
			stream::internal::ContextFreeReader reader(fileStream);
			List<WString> foreignNames;
			vint tdCount = 0;
			vint miCount = 0;
			vint piCount = 0;
			vint eiCount = 0;
			reader << foreignNames << tdCount << miCount << piCount << eiCount;
			TEST_ASSERT(foreignNames.Count() == expectedForeignNames.Count());
			for (vint i = 0; i < foreignNames.Count(); i++)
			{
				TEST_ASSERT(foreignNames[i] == expectedForeignNames[i]);
				if (i > 0)
				{
					TEST_ASSERT(foreignNames[i - 1] < foreignNames[i]);
				}
			}
			TEST_ASSERT(tdCount == localTypeCount);
		}
		TEST_ASSERT(ResetGlobalTypeManager());
	});
}
