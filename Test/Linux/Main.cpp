#include <VlppOS.h>

using namespace vl;
using namespace vl::filesystem;

WString GetTestMetadataPath()
{
    return L"../../Metadata/";
}

TEST_FILE
{
	TEST_CASE_ASSERT(Folder(GetTestMetadataPath()).Exists());
}

int main(int argc, char* argv[])
{
	int result = unittest::UnitTest::RunAndDisposeTests(argc, argv);
	FinalizeGlobalStorage();
	return result;
}
