#include <VlppOS.h>

using namespace vl;
using namespace vl::filesystem;

WString GetTestOutputPath()
{
    return L"../../Output/";
}

TEST_FILE
{
	TEST_CASE_ASSERT(Folder(GetTestOutputPath()).Exists());
}

int main(int argc, char* argv[])
{
	int result = unittest::UnitTest::RunAndDisposeTests(argc, argv);
	FinalizeGlobalStorage();
	return result;
}
