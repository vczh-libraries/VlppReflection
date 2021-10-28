#include "../../Source/Reflection/GuiTypeDescriptorReflection.h"

using namespace vl;
using namespace vl::reflection;
using namespace vl::reflection::description;

namespace TestReflection_TestObjects_Aggregation
{
	class Agg : public Description<Agg>
	{
	public:
		DescriptableObject* Root()
		{
			return GetAggregationRoot();
		}
	};

	class AggParentShared : public Agg, public AggregatableDescription<AggParentShared>
	{
	public:
		~AggParentShared()
		{
			FinalizeAggregation();
		}
	};

	class AggParentRaw : public Agg, public AggregatableDescription<AggParentRaw>
	{
	public:
		~AggParentRaw()
		{
			FinalizeAggregation();
		}
	};

	class AggParentBase : public Agg, public AggregatableDescription<AggParentBase>
	{
	public:
		AggParentBase()
		{
			Ptr<DescriptableObject> shared = new AggParentShared;
			auto raw = new AggParentRaw;

			InitializeAggregation(2);
			SetAggregationParent(0, shared);
			SetAggregationParent(1, raw);
		}

		~AggParentBase()
		{
			FinalizeAggregation();
		}

		AggParentShared* GetParentShared()
		{
			return dynamic_cast<AggParentShared*>(GetAggregationParent(0));
		}

		AggParentRaw* GetParentRaw()
		{
			return dynamic_cast<AggParentRaw*>(GetAggregationParent(1));
		}
	};

	class AggParentDerived : public Agg, public Description<AggParentDerived>
	{
	public:
		AggParentDerived()
		{
			auto base = new AggParentBase;

			InitializeAggregation(1);
			SetAggregationParent(0, base);
		}

		AggParentBase* GetParentBase()
		{
			return dynamic_cast<AggParentBase*>(GetAggregationParent(0));
		}
	};
}
using namespace TestReflection_TestObjects_Aggregation;

#define _ ,

#define TYPE_LIST(F)\
	F(Agg)\
	F(AggParentShared)\
	F(AggParentRaw)\
	F(AggParentBase)\
	F(AggParentDerived)\

BEGIN_TYPE_INFO_NAMESPACE

	TYPE_LIST(DECL_TYPE_INFO)
	TYPE_LIST(IMPL_CPP_TYPE_INFO)

	BEGIN_CLASS_MEMBER(Agg)
	END_CLASS_MEMBER(Agg)

	BEGIN_CLASS_MEMBER(AggParentShared)
	END_CLASS_MEMBER(AggParentShared)

	BEGIN_CLASS_MEMBER(AggParentRaw)
	END_CLASS_MEMBER(AggParentRaw)

	BEGIN_CLASS_MEMBER(AggParentBase)
	END_CLASS_MEMBER(AggParentBase)

	BEGIN_CLASS_MEMBER(AggParentDerived)
	END_CLASS_MEMBER(AggParentDerived)

	class TestTypeLoader_Aggregation : public Object, public ITypeLoader
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

namespace reflection_test_aggregation
{
	void TestDescriptableObjectAggregation()
	{
		{
			auto derived = new AggParentDerived;
			auto base = derived->GetParentBase();
			auto shared = base->GetParentShared();
			auto raw = base->GetParentRaw();

			TEST_ASSERT(derived);
			TEST_ASSERT(base);
			TEST_ASSERT(shared);
			TEST_ASSERT(raw);

			TEST_ASSERT(derived->Root() == nullptr);
			TEST_ASSERT(base->Root() == derived);
			TEST_ASSERT(shared->Root() == derived);
			TEST_ASSERT(raw->Root() == derived);

			auto derivedRC = ReferenceCounterOperator<AggParentDerived>::CreateCounter(derived);
			auto baseRC = ReferenceCounterOperator<AggParentBase>::CreateCounter(base);
			auto sharedRC = ReferenceCounterOperator<AggParentShared>::CreateCounter(shared);
			auto rawRC = ReferenceCounterOperator<AggParentRaw>::CreateCounter(raw);

			TEST_ASSERT(*derivedRC == 0);
			TEST_ASSERT(derivedRC == baseRC);
			TEST_ASSERT(derivedRC == sharedRC);
			TEST_ASSERT(derivedRC == rawRC);

			Ptr<Agg> derivedPtr = derived;
			TEST_ASSERT(*derivedRC == 1);
			{
				Ptr<Agg> basePtr = base;
				TEST_ASSERT(*derivedRC == 2);
				Ptr<Agg> sharedPtr = shared;
				TEST_ASSERT(*derivedRC == 3);
				Ptr<Agg> rawPtr = raw;
				TEST_ASSERT(*derivedRC == 4);
			}
			TEST_ASSERT(*derivedRC == 1);
		}
		{
			auto derived = new AggParentDerived;
			delete derived;
		}
		{
			auto derived = new AggParentDerived;
			delete derived->GetParentBase();
		}
		{
			auto derived = new AggParentDerived;
			delete derived->GetParentBase()->GetParentShared();
		}
		{
			auto derived = new AggParentDerived;
			delete derived->GetParentBase()->GetParentRaw();
		}
		{
			auto derived = new AggParentDerived;
			Ptr<Agg> agg = derived;
		}
		{
			auto derived = new AggParentDerived;
			Ptr<Agg> agg = derived->GetParentBase();
		}
		{
			auto derived = new AggParentDerived;
			Ptr<Agg> agg = derived->GetParentBase()->GetParentShared();
		}
		{
			auto derived = new AggParentDerived;
			Ptr<Agg> agg = derived->GetParentBase()->GetParentRaw();
		}
	}

	void TestDescriptableObjectAggregationCast()
	{
		{
			auto derived = new AggParentDerived;
			auto base = derived->GetParentBase();
			auto shared = base->GetParentShared();
			auto raw = base->GetParentRaw();

			TEST_ASSERT(derived->SafeAggregationCast<AggParentDerived>() == derived);
			TEST_ASSERT(derived->SafeAggregationCast<AggParentBase>() == base);
			TEST_ASSERT(derived->SafeAggregationCast<AggParentShared>() == shared);
			TEST_ASSERT(derived->SafeAggregationCast<AggParentRaw>() == raw);

			TEST_ASSERT(base->SafeAggregationCast<AggParentDerived>() == derived);
			TEST_ASSERT(base->SafeAggregationCast<AggParentBase>() == base);
			TEST_ASSERT(base->SafeAggregationCast<AggParentShared>() == shared);
			TEST_ASSERT(derived->SafeAggregationCast<AggParentRaw>() == raw);

			TEST_ASSERT(shared->SafeAggregationCast<AggParentDerived>() == derived);
			TEST_ASSERT(shared->SafeAggregationCast<AggParentBase>() == base);
			TEST_ASSERT(shared->SafeAggregationCast<AggParentShared>() == shared);
			TEST_ASSERT(shared->SafeAggregationCast<AggParentRaw>() == raw);

			TEST_ASSERT(raw->SafeAggregationCast<AggParentDerived>() == derived);
			TEST_ASSERT(raw->SafeAggregationCast<AggParentBase>() == base);
			TEST_ASSERT(raw->SafeAggregationCast<AggParentShared>() == shared);
			TEST_ASSERT(raw->SafeAggregationCast<AggParentRaw>() == raw);

			TEST_ERROR(derived->SafeAggregationCast<Agg>());
			TEST_ERROR(base->SafeAggregationCast<Agg>());
			TEST_ERROR(shared->SafeAggregationCast<Agg>());
			TEST_ERROR(raw->SafeAggregationCast<Agg>());

			delete derived;
		}
		{
			auto derived = new AggParentDerived;
			auto base = derived->GetParentBase();
			auto shared = base->GetParentShared();
			auto raw = base->GetParentRaw();

			{
				auto value = Value::From(derived);
				TEST_ASSERT(UnboxValue<AggParentDerived*>(value) == derived);
				TEST_ASSERT(UnboxValue<AggParentBase*>(value) == base);
				TEST_ASSERT(UnboxValue<AggParentShared*>(value) == shared);
				TEST_ASSERT(UnboxValue<AggParentRaw*>(value) == raw);
				TEST_ERROR(UnboxValue<Agg*>(value));
			}
			{
				auto value = Value::From(base);
				TEST_ASSERT(UnboxValue<AggParentDerived*>(value) == derived);
				TEST_ASSERT(UnboxValue<AggParentBase*>(value) == base);
				TEST_ASSERT(UnboxValue<AggParentShared*>(value) == shared);
				TEST_ASSERT(UnboxValue<AggParentRaw*>(value) == raw);
				TEST_ERROR(UnboxValue<Agg*>(value));
			}
			{
				auto value = Value::From(shared);
				TEST_ASSERT(UnboxValue<AggParentDerived*>(value) == derived);
				TEST_ASSERT(UnboxValue<AggParentBase*>(value) == base);
				TEST_ASSERT(UnboxValue<AggParentShared*>(value) == shared);
				TEST_ASSERT(UnboxValue<AggParentRaw*>(value) == raw);
				TEST_ERROR(UnboxValue<Agg*>(value));
			}
			{
				auto value = Value::From(raw);
				TEST_ASSERT(UnboxValue<AggParentDerived*>(value) == derived);
				TEST_ASSERT(UnboxValue<AggParentBase*>(value) == base);
				TEST_ASSERT(UnboxValue<AggParentShared*>(value) == shared);
				TEST_ASSERT(UnboxValue<AggParentRaw*>(value) == raw);
				TEST_ERROR(UnboxValue<Agg*>(value));
			}

			delete derived;
		}
		{
			auto derived = new AggParentDerived;
			auto base = derived->GetParentBase();
			auto shared = base->GetParentShared();
			auto raw = base->GetParentRaw();

			Ptr<Agg> ptr = derived;
			{
				auto value = Value::From(ptr);
				TEST_ASSERT(UnboxValue<Ptr<AggParentDerived>>(value) == derived);
				TEST_ASSERT(UnboxValue<Ptr<AggParentBase>>(value) == base);
				TEST_ASSERT(UnboxValue<Ptr<AggParentShared>>(value) == shared);
				TEST_ASSERT(UnboxValue<Ptr<AggParentRaw>>(value) == raw);
				TEST_ERROR(UnboxValue<Ptr<Agg>>(value));
			}
			{
				auto value = Value::From(ptr);
				TEST_ASSERT(UnboxValue<Ptr<AggParentDerived>>(value) == derived);
				TEST_ASSERT(UnboxValue<Ptr<AggParentBase>>(value) == base);
				TEST_ASSERT(UnboxValue<Ptr<AggParentShared>>(value) == shared);
				TEST_ASSERT(UnboxValue<Ptr<AggParentRaw>>(value) == raw);
				TEST_ERROR(UnboxValue<Ptr<Agg>>(value));
			}
			{
				auto value = Value::From(ptr);
				TEST_ASSERT(UnboxValue<Ptr<AggParentDerived>>(value) == derived);
				TEST_ASSERT(UnboxValue<Ptr<AggParentBase>>(value) == base);
				TEST_ASSERT(UnboxValue<Ptr<AggParentShared>>(value) == shared);
				TEST_ASSERT(UnboxValue<Ptr<AggParentRaw>>(value) == raw);
				TEST_ERROR(UnboxValue<Ptr<Agg>>(value));
			}
			{
				auto value = Value::From(ptr);
				TEST_ASSERT(UnboxValue<Ptr<AggParentDerived>>(value) == derived);
				TEST_ASSERT(UnboxValue<Ptr<AggParentBase>>(value) == base);
				TEST_ASSERT(UnboxValue<Ptr<AggParentShared>>(value) == shared);
				TEST_ASSERT(UnboxValue<Ptr<AggParentRaw>>(value) == raw);
				TEST_ERROR(UnboxValue<Ptr<Agg>>(value));
			}
		}
	}

	void TestDescriptableObjectIsAggregation()
	{
		TEST_ASSERT(GetTypeDescriptor<AggParentShared>()->IsAggregatable() == true);
		TEST_ASSERT(GetTypeDescriptor<AggParentRaw>()->IsAggregatable() == true);
		TEST_ASSERT(GetTypeDescriptor<AggParentBase>()->IsAggregatable() == true);
		TEST_ASSERT(GetTypeDescriptor<AggParentDerived>()->IsAggregatable() == false);
	}
}
using namespace reflection_test_aggregation;

#define TEST_CASE_REFLECTION(NAME)\
	TEST_CASE(L ## #NAME)\
	{\
		TEST_ASSERT(LoadPredefinedTypes());\
		TEST_ASSERT(GetGlobalTypeManager()->AddTypeLoader(new TestTypeLoader_Aggregation));\
		TEST_ASSERT(GetGlobalTypeManager()->Load());\
		{\
			NAME();\
		}\
		TEST_ASSERT(ResetGlobalTypeManager());\
	});\

TEST_FILE
{
	TEST_CASE_REFLECTION(TestDescriptableObjectAggregation)
	TEST_CASE_REFLECTION(TestDescriptableObjectAggregationCast)
	TEST_CASE_REFLECTION(TestDescriptableObjectIsAggregation)
}
