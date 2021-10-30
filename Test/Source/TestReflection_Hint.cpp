#include "../../Source/Reflection/Reflection/Reflection.h"

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

	class HintTester1 :public Description<HintTester1>
	{
	public:
		Func<int(int)> GetFunc(Func<int(int)> x) { throw 0; }
		Ptr<HintTester1> GetHintTester(Ptr<HintTester1> x) { throw 0; }
		vint GetInt(vint x) { throw 0; }
	};

	class HintTester2 :public Description<HintTester2>
	{
	public:
		const LazyList<int>& GetLazyList(LazyList<int>& x) { throw 0; }
		const Array<int>& GetArray(Array<int>& x) { throw 0; }
		const List<int>& GetList(List<int>& x) { throw 0; }
		const SortedList<int>& GetSortedList(SortedList<int>& x) { throw 0; }
		const ObservableList<vint>& GetObservableList() { throw nullptr; }
		const Dictionary<int, int>& GetDictionary(Dictionary<int, int>& x) { throw 0; }
		const MyList& GetMyList(MyList& x) { throw 0; }
		const MyObservableList& GetMyObservableList(MyObservableList& x) { throw 0; }
	};

	class HintTester3 :public Description<HintTester3>
	{
	public:
		LazyList<int>& GetLazyList(const LazyList<int>& x) { throw 0; }
		Array<int>& GetArray(const Array<int>& x) { throw 0; }
		List<int>& GetList(const List<int>& x) { throw 0; }
		SortedList<int>& GetSortedList(const SortedList<int>& x) { throw 0; }
		ObservableList<vint>& GetObservableList() { throw nullptr; }
		Dictionary<int, int>& GetDictionary(const Dictionary<int, int>& x) { throw 0; }
		MyList& GetMyList(const MyList& x) { throw 0; }
		MyObservableList& GetMyObservableList(const MyObservableList& x) { throw 0; }
	};
}
using namespace TestReflection_TestObjects_Hint;

#define _ ,

#define TYPE_LIST(F)\
	F(HintTester1)\
	F(HintTester2)\
	F(HintTester3)\

BEGIN_TYPE_INFO_NAMESPACE

	TYPE_LIST(DECL_TYPE_INFO)
	TYPE_LIST(IMPL_CPP_TYPE_INFO)

	BEGIN_CLASS_MEMBER(HintTester1)
		CLASS_MEMBER_METHOD(GetFunc, { L"x" })
		CLASS_MEMBER_METHOD(GetHintTester, { L"x" })
		CLASS_MEMBER_METHOD(GetInt, { L"x" })
	END_CLASS_MEMBER(HintTester1)

	BEGIN_CLASS_MEMBER(HintTester2)
		CLASS_MEMBER_METHOD(GetLazyList, {L"x"})
		CLASS_MEMBER_METHOD(GetArray, { L"x" })
		CLASS_MEMBER_METHOD(GetList, { L"x" })
		CLASS_MEMBER_METHOD(GetSortedList, { L"x" })
		CLASS_MEMBER_METHOD(GetObservableList, NO_PARAMETER)
		CLASS_MEMBER_METHOD(GetDictionary, { L"x" })
		CLASS_MEMBER_METHOD(GetMyList, { L"x" })
		CLASS_MEMBER_METHOD(GetMyObservableList, { L"x" })
	END_CLASS_MEMBER(HintTester2)

	BEGIN_CLASS_MEMBER(HintTester3)
		CLASS_MEMBER_METHOD(GetLazyList, {L"x"})
		CLASS_MEMBER_METHOD(GetArray, { L"x" })
		CLASS_MEMBER_METHOD(GetList, { L"x" })
		CLASS_MEMBER_METHOD(GetSortedList, { L"x" })
		CLASS_MEMBER_METHOD(GetObservableList, NO_PARAMETER)
		CLASS_MEMBER_METHOD(GetDictionary, { L"x" })
		CLASS_MEMBER_METHOD(GetMyList, { L"x" })
		CLASS_MEMBER_METHOD(GetMyObservableList, { L"x" })
	END_CLASS_MEMBER(HintTester3)

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
	template<typename THintTester, typename TReturn, typename TArgument>
	void TestHint(const WString& member, TypeInfoHint hint, bool testParameter = true)
	{
		auto td = GetTypeDescriptor<THintTester>();
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

	void TestHint1()
	{
		TestHint<HintTester1, IValueFunctionProxy, IValueFunctionProxy>(L"GetFunc", TypeInfoHint::Normal);
		TestHint<HintTester1, HintTester1, HintTester1>(L"GetHintTester", TypeInfoHint::Normal);
		TestHint<HintTester1, vint, vint>(L"GetInt", TypeInfoHint::Normal);
	}

	void TestHint2()
	{
		TestHint<HintTester2, IValueEnumerable, IValueEnumerable>(L"GetLazyList", TypeInfoHint::LazyList);
		TestHint<HintTester2, IValueReadonlyList, IValueArray>(L"GetArray", TypeInfoHint::Array);
		TestHint<HintTester2, IValueReadonlyList, IValueList>(L"GetList", TypeInfoHint::List);
		TestHint<HintTester2, IValueReadonlyList, IValueReadonlyList>(L"GetSortedList", TypeInfoHint::SortedList);
		TestHint<HintTester2, IValueReadonlyList, void>(L"GetObservableList", TypeInfoHint::ObservableList, false);
		TestHint<HintTester2, IValueReadonlyDictionary, IValueDictionary>(L"GetDictionary", TypeInfoHint::Dictionary);
		TestHint<HintTester2, IValueReadonlyList, IValueList>(L"GetMyList", TypeInfoHint::NativeCollectionReference);
		TestHint<HintTester2, IValueReadonlyList, IValueList>(L"GetMyObservableList", TypeInfoHint::NativeCollectionReference);
	}

	void TestHint3()
	{
		TestHint<HintTester3, IValueEnumerable, IValueEnumerable>(L"GetLazyList", TypeInfoHint::LazyList);
		TestHint<HintTester3, IValueArray, IValueReadonlyList>(L"GetArray", TypeInfoHint::Array);
		TestHint<HintTester3, IValueList, IValueReadonlyList>(L"GetList", TypeInfoHint::List);
		TestHint<HintTester3, IValueReadonlyList, IValueReadonlyList>(L"GetSortedList", TypeInfoHint::SortedList);
		TestHint<HintTester3, IValueObservableList, void>(L"GetObservableList", TypeInfoHint::ObservableList, false);
		TestHint<HintTester3, IValueDictionary, IValueReadonlyDictionary>(L"GetDictionary", TypeInfoHint::Dictionary);
		TestHint<HintTester3, IValueList, IValueReadonlyList>(L"GetMyList", TypeInfoHint::NativeCollectionReference);
		TestHint<HintTester3, IValueList, IValueReadonlyList>(L"GetMyObservableList", TypeInfoHint::NativeCollectionReference);
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
	TEST_CASE_REFLECTION(TestHint1)
	TEST_CASE_REFLECTION(TestHint2)
	TEST_CASE_REFLECTION(TestHint3)
}
