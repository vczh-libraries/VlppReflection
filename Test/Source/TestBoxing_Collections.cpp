#include "../../Source/Reflection/GuiTypeDescriptorReflection.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection;
using namespace vl::reflection::description;

namespace reflection_test_boxunboxcollections
{
	template<typename TValueItf, typename TCollection>
	Ptr<TValueItf> UnboxCollection(Ptr<TCollection> cs)
	{
		const auto& cref = *cs.Obj();
		TValueItf* baseline = nullptr;
		{
			auto xs1 = UnboxValue<Ptr<TValueItf>>(BoxParameter(cref));
			auto xs2 = UnboxValue<Ptr<TValueItf>>(BoxParameter(cref));
			TEST_ASSERT(xs1);
			TEST_ASSERT(xs1 == xs2);
			baseline = xs1.Obj();
		}
		{
			auto xs1 = UnboxValue<Ptr<TValueItf>>(BoxParameter(cref));
			auto xs2 = UnboxValue<Ptr<TValueItf>>(BoxParameter(cref));
			TEST_ASSERT(xs1 == baseline);
			TEST_ASSERT(xs1 == xs2);
		}
		return baseline;
	}

	void TestLazyList()
	{
		LazyList<vint> ll;
		{
			auto cs = MakePtr<List<vint>>();
			cs->Add(0);
			cs->Add(1);
			cs->Add(2);
			ll = cs;
		}
		auto xs1 = UnboxValue<Ptr<IValueEnumerable>>(BoxParameter(ll));
		auto xs2 = UnboxValue<Ptr<IValueEnumerable>>(BoxParameter(ll));
		TEST_ASSERT(xs1 != xs2);

		ll = {};
		auto xs3 = UnboxValue<Ptr<IValueEnumerable>>(BoxParameter(ll));

		LazyList<vint> l1, l2, l3;
		UnboxParameter(BoxValue(xs1), l1);
		UnboxParameter(BoxValue(xs2), l2);
		UnboxParameter(BoxValue(xs3), l3);

		TEST_ASSERT(l1.Count() == 3);
		TEST_ASSERT(l2.Count() == 3);
		TEST_ASSERT(l3.Count() == 0);
	}

	void TestArray()
	{
		auto cs = MakePtr<Array<vint>>();
		auto xs = UnboxCollection<IValueArray>(cs);

		xs->Resize(1);
		xs->Set(0, BoxValue<vint>(100));
		TEST_ASSERT(UnboxValue<vint>(xs->Get(0)) == 100);

		{
			Array<vint> rs;
			UnboxParameter(BoxValue(xs), rs);
			TEST_ASSERT(rs[0] == 100);
		}
		cs = nullptr;
		TEST_EXCEPTION(xs->CreateEnumerator(), ObjectDisposedException, [](auto) {});
	}

	void TestList()
	{
		auto cs = MakePtr<List<vint>>();
		auto xs = UnboxCollection<IValueList>(cs);

		xs->Add(BoxValue<vint>(100));
		TEST_ASSERT(UnboxValue<vint>(xs->Get(0)) == 100);

		{
			List<vint> rs;
			UnboxParameter(BoxValue(xs), rs);
			TEST_ASSERT(rs[0] == 100);
		}
		cs = nullptr;
		TEST_EXCEPTION(xs->CreateEnumerator(), ObjectDisposedException, [](auto) {});
	}

	void TestSortedList()
	{
		auto cs = MakePtr<SortedList<vint>>();
		auto xs = UnboxCollection<IValueReadonlyList>(cs);

		cs->Add(100);
		TEST_ASSERT(UnboxValue<vint>(xs->Get(0)) == 100);

		{
			SortedList<vint> rs;
			UnboxParameter(BoxValue(xs), rs);
			TEST_ASSERT(rs[0] == 100);
		}
		cs = nullptr;
		TEST_EXCEPTION(xs->CreateEnumerator(), ObjectDisposedException, [](auto) {});
	}

	void TestDictionary()
	{
		auto cs = MakePtr<Dictionary<vint, vint>>();
		auto xs = UnboxCollection<IValueDictionary>(cs);

		xs->Set(BoxValue<vint>(100), BoxValue<vint>(200));
		TEST_ASSERT(UnboxValue<vint>(xs->Get(BoxValue<vint>(100))) == 200);

		{
			Dictionary<vint, vint> rs;
			UnboxParameter(BoxValue(xs), rs);
			TEST_ASSERT(rs[100] == 200);
		}
		cs = nullptr;
		TEST_EXCEPTION(xs->Clear(), ObjectDisposedException, [](auto) {});
	}

	void TestObservableList()
	{
		auto cs = MakePtr<ObservableList<vint>>();
		auto xs = UnboxCollection<IValueObservableList>(cs);

		cs->Add(100);
		TEST_ASSERT(UnboxValue<vint>(xs->Get(0)) == 100);

		{
			ObservableList<vint> rs;
			ParameterAccessor<ObservableList<vint>, TypeFlags::ObservableListType>::UnboxParameter(BoxValue(xs), rs, nullptr, L"");
			TEST_ASSERT(rs[0] == 100);
		}
		cs = nullptr;
		TEST_EXCEPTION(xs->CreateEnumerator(), ObjectDisposedException, [](auto) {});
	}
}
using namespace reflection_test_boxunboxcollections;

#define TEST_CASE_REFLECTION(NAME)\
	TEST_CASE(L ## #NAME)\
	{\
		TEST_ASSERT(LoadPredefinedTypes());\
		TEST_ASSERT(GetGlobalTypeManager()->Load());\
		{\
			NAME();\
		}\
		TEST_ASSERT(ResetGlobalTypeManager());\
	});\

TEST_FILE
{
	TEST_CASE_REFLECTION(TestLazyList)
	TEST_CASE_REFLECTION(TestArray)
	TEST_CASE_REFLECTION(TestList)
	TEST_CASE_REFLECTION(TestSortedList)
	TEST_CASE_REFLECTION(TestDictionary)
	TEST_CASE_REFLECTION(TestObservableList)
}
