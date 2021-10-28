#include "../../Source/Reflection/GuiTypeDescriptorReflection.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection;
using namespace vl::reflection::description;

namespace TestReflection_TestObjects_Hint
{
	class MyList : public List<int>
	{
	};

	class MyObservableList : public ObservableListBase<vint>
	{
	};

	class HintTester :public Description<HintTester>
	{
	public:
		LazyList<int> GetLazyList(const LazyList<int>& x) { return x; }
		const Array<int>& GetArray(Array<int>& x) { return x; }
		const List<int>& GetList(List<int>& x) { return x; }
		const SortedList<int>& GetSortedList(SortedList<int>& x) { return x; }
		const ObservableList<vint>& GetReadableObservableList() { throw nullptr; }
		ObservableList<vint>& GetObservableList() { throw nullptr; }
		const Dictionary<int, int>& GetDictionary(Dictionary<int, int>& x) { return x; }
		const MyList& GetMyList(MyList& x) { return x; }
		const MyObservableList& GetMyObservableList(MyObservableList& x) { return x; }
		Func<int(int)> GetFunc(Func<int(int)> x) { return x; }
		Ptr<HintTester> GetHintTester(Ptr<HintTester> x) { return x; }
		vint GetInt(vint x) { return x; }
	};
}
using namespace TestReflection_TestObjects_Hint;

#define _ ,

#define TYPE_LIST(F)\
	F(HintTester)\

BEGIN_TYPE_INFO_NAMESPACE

	TYPE_LIST(DECL_TYPE_INFO)
	TYPE_LIST(IMPL_CPP_TYPE_INFO)

	BEGIN_CLASS_MEMBER(HintTester)
		CLASS_MEMBER_METHOD(GetLazyList, {L"x"})
		CLASS_MEMBER_METHOD(GetArray, { L"x" })
		CLASS_MEMBER_METHOD(GetList, { L"x" })
		CLASS_MEMBER_METHOD(GetSortedList, { L"x" })
		CLASS_MEMBER_METHOD(GetReadableObservableList, NO_PARAMETER)
		CLASS_MEMBER_METHOD(GetObservableList, NO_PARAMETER)
		CLASS_MEMBER_METHOD(GetDictionary, { L"x" })
		CLASS_MEMBER_METHOD(GetMyList, { L"x" })
		CLASS_MEMBER_METHOD(GetMyObservableList, { L"x" })
		CLASS_MEMBER_METHOD(GetFunc, { L"x" })
		CLASS_MEMBER_METHOD(GetHintTester, { L"x" })
		CLASS_MEMBER_METHOD(GetInt, { L"x" })
	END_CLASS_MEMBER(HintTester)

	class TestTypeLoader_Hint : public Object, public ITypeLoader
	{
	public:
		void Load(ITypeManager* manager)override
		{
			TYPE_LIST(ADD_TYPE_INFO)
		}
		
		void Unload(ITypeManager* manager)override
		{
		}
	};

END_TYPE_INFO_NAMESPACE

#undef TYPE_LIST

namespace reflection_test_hint
{
	template<typename TReturn, typename TArgument>
	void TestHint(const WString& member, TypeInfoHint hint, bool testParameter = true)
	{
		auto td = GetTypeDescriptor<HintTester>();
		auto method = td->GetMethodGroupByName(member, false)->GetMethod(0);
		TEST_ASSERT(method->GetReturn()->GetTypeDescriptor() == GetTypeDescriptor<TReturn>());
		TEST_ASSERT(method->GetReturn()->GetHint() == hint);

		if (testParameter)
		{
			TEST_ASSERT(method->GetParameterCount() == 1);
			TEST_ASSERT(method->GetParameter(0)->GetType()->GetTypeDescriptor() == GetTypeDescriptor<TArgument>());
			TEST_ASSERT(method->GetParameter(0)->GetType()->GetHint() == hint);
		}
	}

	void TestHint()
	{
		TestHint<IValueEnumerable, IValueEnumerable>(L"GetLazyList", TypeInfoHint::LazyList);
		TestHint<IValueReadonlyList, IValueArray>(L"GetArray", TypeInfoHint::Array);
		TestHint<IValueReadonlyList, IValueList>(L"GetList", TypeInfoHint::List);
		TestHint<IValueReadonlyList, IValueReadonlyList>(L"GetSortedList", TypeInfoHint::SortedList);
		TestHint<IValueReadonlyList, void>(L"GetReadableObservableList", TypeInfoHint::ObservableList, false);
		TestHint<IValueObservableList, void>(L"GetObservableList", TypeInfoHint::ObservableList, false);
		TestHint<IValueReadonlyDictionary, IValueDictionary>(L"GetDictionary", TypeInfoHint::Dictionary);
		TestHint<IValueReadonlyList, IValueList>(L"GetMyList", TypeInfoHint::NativeCollectionReference);
		TestHint<IValueReadonlyList, IValueList>(L"GetMyObservableList", TypeInfoHint::NativeCollectionReference);
		TestHint<IValueFunctionProxy, IValueFunctionProxy>(L"GetFunc", TypeInfoHint::Normal);
		TestHint<HintTester, HintTester>(L"GetHintTester", TypeInfoHint::Normal);
		TestHint<vint, vint>(L"GetInt", TypeInfoHint::Normal);
	}
}
using namespace reflection_test_hint;

#define TEST_CASE_REFLECTION(NAME)\
	TEST_CASE(L ## #NAME)\
	{\
		TEST_ASSERT(LoadPredefinedTypes());\
		TEST_ASSERT(GetGlobalTypeManager()->AddTypeLoader(new TestTypeLoader_Hint));\
		TEST_ASSERT(GetGlobalTypeManager()->Load());\
		{\
			NAME();\
		}\
		TEST_ASSERT(ResetGlobalTypeManager());\
	});\

TEST_FILE
{
	TEST_CASE_REFLECTION(TestHint)
}
