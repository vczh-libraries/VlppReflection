#include <VlppOS.h>
#include <windows.h>

using namespace vl;
using namespace vl::filesystem;

WString GetExePath()
{
	wchar_t buffer[65536];
	GetModuleFileName(NULL, buffer, sizeof(buffer)/sizeof(*buffer));
	vint pos=-1;
	vint index=0;
	while(buffer[index])
	{
		if(buffer[index]==L'\\' || buffer[index]==L'/')
		{
			pos=index;
		}
		index++;
	}
	return WString::CopyFrom(buffer, pos + 1);
}

WString GetTestMetadataPath()
{
#ifdef _WIN64
	return GetExePath() + L"../../../Metadata/";
#else
	return GetExePath() + L"../../Metadata/";
#endif
}

TEST_FILE
{
	TEST_CASE_ASSERT(Folder(GetTestMetadataPath()).Exists());
}

int wmain(int argc, wchar_t* argv[])
{
	{
		Folder folder(GetTestMetadataPath());
		if (!folder.Exists())
		{
			folder.Create(false);
		}
	}
	int result = unittest::UnitTest::RunAndDisposeTests(argc, argv);
	FinalizeGlobalStorage();
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}