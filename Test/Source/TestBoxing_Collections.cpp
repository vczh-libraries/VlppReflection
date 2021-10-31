#include "Common.h"

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

		auto l1 = UnboxParameter<LazyList<vint>>(BoxValue(xs1));
		auto l2 = UnboxParameter<LazyList<vint>>(BoxValue(xs2));
		auto l3 = UnboxParameter<LazyList<vint>>(BoxValue(xs3));

		TEST_ASSERT(l1.Ref().Count() == 3);
		TEST_ASSERT(l2.Ref().Count() == 3);
		TEST_ASSERT(l3.Ref().Count() == 0);
	}

	void TestArray()
	{
		auto cs = MakePtr<Array<vint>>();
		auto xs = UnboxCollection<IValueArray>(cs);

		xs->Resize(1);
		xs->Set(0, BoxValue<vint>(100));
		TEST_ASSERT(UnboxValue<vint>(xs->Get(0)) == 100);

		{
			auto rs = UnboxParameter<Array<vint>>(BoxValue(xs));
			TEST_ASSERT(rs.Ref()[0] == 100);
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
			auto rs = UnboxParameter<List<vint>>(BoxValue(xs));
			TEST_ASSERT(rs.Ref()[0] == 100);
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
			auto rs = UnboxParameter<SortedList<vint>>(BoxValue(xs));
			TEST_ASSERT(rs.Ref()[0] == 100);
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
			auto rs = UnboxParameter<Dictionary<vint, vint>>(BoxValue(xs));
			TEST_ASSERT(rs.Ref()[100] == 200);
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
			auto rs = UnboxParameter<ObservableList<vint>>(BoxValue(xs), nullptr, L"");
			TEST_ASSERT(rs.Ref()[0] == 100);
		}
		cs = nullptr;
		TEST_EXCEPTION(xs->CreateEnumerator(), ObjectDisposedException, [](auto) {});
	}

	void TestBoxingThenUnboxing()
	{
		List<vint> xs;
		Array<vint> ys;
		{
			vint zs[] = { 1,2,3 };
			CopyFrom(xs, From(zs));
			CopyFrom(ys, From(zs));
		}

		auto xrv = UnboxValue<Ptr<IValueReadonlyList>>(BoxParameter(xs));
		auto yrv = UnboxValue<Ptr<IValueReadonlyList>>(BoxParameter(ys));
		auto xv = xrv.Cast<IValueList>();
		auto yv = yrv.Cast<IValueArray>();

		TEST_ASSERT(xrv);
		TEST_ASSERT(yrv);
		TEST_ASSERT(xv);
		TEST_ASSERT(yv);

		xv->Add(BoxValue<vint>(4));
		yv->Resize(4);
		yv->Set(3, BoxValue<vint>(5));

		TEST_ASSERT(xs.Count() == 4);
		TEST_ASSERT(xs[0] == 1);
		TEST_ASSERT(xs[1] == 2);
		TEST_ASSERT(xs[2] == 3);
		TEST_ASSERT(xs[3] == 4);

		TEST_ASSERT(ys.Count() == 4);
		TEST_ASSERT(ys[0] == 1);
		TEST_ASSERT(ys[1] == 2);
		TEST_ASSERT(ys[2] == 3);
		TEST_ASSERT(ys[3] == 5);

		auto xu1 = UnboxParameter<List<vint>>(BoxValue(xrv));
		auto yu1 = UnboxParameter<Array<vint>>(BoxValue(yrv));
		auto xu2 = UnboxParameter<SortedList<vint>>(BoxValue(xrv));
		auto yu2 = UnboxParameter<SortedList<vint>>(BoxValue(yrv));

		TEST_ASSERT(&xu1.Ref() == &xs);
		TEST_ASSERT(&yu1.Ref() == &ys);

		TEST_ASSERT(xu1.IsOwned() == false);
		TEST_ASSERT(yu1.IsOwned() == false);
		TEST_ASSERT(xu2.IsOwned() == true);
		TEST_ASSERT(yu2.IsOwned() == true);

		xu2.Ref().Remove(0);
		yu2.Ref().Remove(0);
		TEST_ASSERT(xs.Count() == 3);
		TEST_ASSERT(ys.Count() == 3);
	}
}
using namespace reflection_test_boxunboxcollections;

#define TEST_CASE_REFLECTION(NAME) TEST_CASE_REFLECTION_NOLOADER(NAME)

TEST_FILE
{
	TEST_CASE_REFLECTION(TestLazyList)
	TEST_CASE_REFLECTION(TestArray)
	TEST_CASE_REFLECTION(TestList)
	TEST_CASE_REFLECTION(TestSortedList)
	TEST_CASE_REFLECTION(TestDictionary)
	TEST_CASE_REFLECTION(TestObservableList)
	TEST_CASE_REFLECTION(TestBoxingThenUnboxing)
}
