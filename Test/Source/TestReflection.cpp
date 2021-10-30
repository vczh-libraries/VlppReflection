#include "../../Source/Reflection/Reflection/Reflection.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection;
using namespace vl::reflection::description;

namespace TestReflection_TestObjects
{
	enum Season
	{
		Spring,
		Summer,
		Autumn,
		Winter,
	};

	enum ResetOption
	{
		ResetNone=0,
		ResetA=1,
		ResetB=2,
	};

	struct Point
	{
		vint x;
		vint y;
	};

	struct Size
	{
		vint cx;
		vint cy;
	};

	struct Rect
	{
		Point point;
		Size size;
	};

	struct RectPair
	{
		Rect a;
		Rect b;
	};

	class Base : public Object, public Description<Base>
	{
	public:
		vint a;
		Season season;
		Base():a(0), season(Spring){}
		Base(vint _a):a(_a){}
		static Ptr<Base> Create(vint _a, vint _b){return new Base(_a+_b);}
	};

	class Derived : public Base, public Description<Derived>
	{
	private:
		vint b;
	public:
		Derived():b(0){}
		Derived(vint _a, vint _b):Base(_a),b(_b){}
		static Ptr<Derived> Create(){return new Derived();}
		static Ptr<Derived> Create(vint _a, vint _b){return new Derived(_a, _b);}

		vint GetB(){return b;}
		void SetB(vint value){b=value;}
		void Reset(){a=0; b=0;}
		void Reset(vint _a, vint _b){a=_a; b=_b;}
		void Reset2(ResetOption opt){if(opt&ResetA) a=0; if(opt&ResetB) b=0;}
		static void Reset3(Derived* _this, Derived* derived){_this->a=derived->a; _this->b=derived->b;}
		
		Nullable<WString> c;
		Nullable<WString> GetC(){return c;}
		void SetC(Nullable<WString> value){c=value;}
	};

	class BaseSummer : public Description<BaseSummer>
	{
	protected:
		Array<Ptr<Base>>		bases;
	public:

		const Array<Ptr<Base>>& GetBases()
		{
			return bases;
		}

		void SetBases(const Array<Ptr<Base>>& _bases)
		{
			CopyFrom(bases, _bases);
		}

		vint Sum()
		{
			return From(bases)
				.Select([](auto&& base){return base->a;})
				.Aggregate((vint)0, [](vint a, vint b){return a+b;});
		}

		List<Ptr<Base>>			bases3;

		List<Ptr<Base>>& GetBases2()
		{
			return bases3;
		}

		void SetBases2(List<Ptr<Base>>& _bases)
		{
			CopyFrom(bases3, _bases);
		}

		vint Sum2()
		{
			return From(bases3)
				.Select([](auto&& base){return base->a;})
				.Aggregate((vint)0, [](vint a, vint b){return a+b;});
		}

		Func<vint(vint)> Sum3(Func<vint(vint)> f1, Func<vint(vint)> f2)
		{
			return [=](vint i){return f1(i)+f2(i);};
		}
	};

	class DictionaryHolder : public Description<DictionaryHolder>
	{
	public:
		Dictionary<vint, WString>				maps;
		Dictionary<Ptr<Base>, Ptr<Base>>		maps2;

		const Dictionary<vint, WString>& GetMaps()
		{ 
			return maps;
		}

		void SetMaps(const Dictionary<vint, WString>& value)
		{
			CopyFrom(maps, value);
		}
	};
}
using namespace TestReflection_TestObjects;

#define _ ,

#define TYPE_LIST(F)\
	F(Season)\
	F(ResetOption)\
	F(Base)\
	F(Derived)\
	F(BaseSummer)\
	F(DictionaryHolder)\
	F(Point)\
	F(Size)\
	F(Rect)\
	F(RectPair)\

BEGIN_TYPE_INFO_NAMESPACE

	TYPE_LIST(DECL_TYPE_INFO)
	TYPE_LIST(IMPL_CPP_TYPE_INFO)

	BEGIN_ENUM_ITEM(Season)
		ENUM_ITEM(Spring)
		ENUM_ITEM(Summer)
		ENUM_ITEM(Autumn)
		ENUM_ITEM(Winter)
	END_ENUM_ITEM(Season)

	BEGIN_ENUM_ITEM_MERGABLE(ResetOption)
		ENUM_ITEM(ResetNone)
		ENUM_ITEM(ResetA)
		ENUM_ITEM(ResetB)
	END_ENUM_ITEM(ResetOption)

	BEGIN_CLASS_MEMBER(Base)
		CLASS_MEMBER_FIELD(a)
		CLASS_MEMBER_FIELD(season)
		CLASS_MEMBER_CONSTRUCTOR(Ptr<Base>(), NO_PARAMETER)
		CLASS_MEMBER_CONSTRUCTOR(Ptr<Base>(vint), {L"_a"})
		CLASS_MEMBER_EXTERNALCTOR(Ptr<Base>(vint, vint), {L"_a" _ L"_b"}, Base::Create)
	END_CLASS_MEMBER(Base)

	BEGIN_CLASS_MEMBER(Derived)
		CLASS_MEMBER_BASE(Base)
		CLASS_MEMBER_CONSTRUCTOR(Ptr<Derived>(), NO_PARAMETER)
		CLASS_MEMBER_CONSTRUCTOR(Ptr<Derived>(vint _ vint), {L"_a" _ L"_b"})

		CLASS_MEMBER_STATIC_METHOD_OVERLOAD(Create, NO_PARAMETER, Ptr<Derived>(*)())
		CLASS_MEMBER_STATIC_METHOD_OVERLOAD(Create, {L"_a" _ L"_b"}, Ptr<Derived>(*)(vint _ vint))

		CLASS_MEMBER_METHOD(GetB, NO_PARAMETER)
		CLASS_MEMBER_METHOD(SetB, {L"value"})
		CLASS_MEMBER_PROPERTY(b, GetB, SetB)

		CLASS_MEMBER_METHOD_OVERLOAD(Reset, NO_PARAMETER, void(Derived::*)())
		CLASS_MEMBER_METHOD_OVERLOAD(Reset, {L"_a" _ L"_b"}, void(Derived::*)(vint _ vint))
		CLASS_MEMBER_METHOD_RENAME(Reset, Reset2, {L"opt"})
		CLASS_MEMBER_EXTERNALMETHOD(Reset, {L"derived"}, void(Derived::*)(Derived*), Derived::Reset3)

		CLASS_MEMBER_FIELD(c)
		CLASS_MEMBER_PROPERTY_FAST(C)
	END_CLASS_MEMBER(Derived)

	BEGIN_STRUCT_MEMBER(Point)
		STRUCT_MEMBER(x)
		STRUCT_MEMBER(y)
	END_STRUCT_MEMBER(Point)

	BEGIN_STRUCT_MEMBER(Size)
		STRUCT_MEMBER(cx)
		STRUCT_MEMBER(cy)
	END_STRUCT_MEMBER(Size)

	BEGIN_STRUCT_MEMBER(Rect)
		STRUCT_MEMBER(point)
		STRUCT_MEMBER(size)
	END_STRUCT_MEMBER(Rect)

	BEGIN_STRUCT_MEMBER(RectPair)
		STRUCT_MEMBER(a)
		STRUCT_MEMBER(b)
	END_STRUCT_MEMBER(RectPair)

	Ptr<IValueReadonlyList> BaseSummer_GetBases(BaseSummer* thisObject)
	{
		return new ValueReadonlyListWrapper<const Array<Ptr<Base>>*>(&thisObject->GetBases());
	}

	void BaseSummer_SetBases(BaseSummer* thisObject, Ptr<IValueReadonlyList> bases)
	{
		Array<Ptr<Base>> baseArray;
		CopyFrom(baseArray, GetLazyList<Ptr<Base>>(bases));
		thisObject->SetBases(baseArray);
	}

	BEGIN_CLASS_MEMBER(BaseSummer)
		CLASS_MEMBER_CONSTRUCTOR(Ptr<BaseSummer>(), NO_PARAMETER)
		CLASS_MEMBER_METHOD(Sum, NO_PARAMETER)
		CLASS_MEMBER_METHOD(Sum2, NO_PARAMETER)
		CLASS_MEMBER_METHOD(Sum3, {L"f1" _ L"f2"})
		CLASS_MEMBER_METHOD(GetBases, NO_PARAMETER)
		CLASS_MEMBER_METHOD(SetBases, {L"bases"})
		CLASS_MEMBER_METHOD(GetBases2, NO_PARAMETER)
		CLASS_MEMBER_METHOD(SetBases2, {L"bases"})
		CLASS_MEMBER_FIELD(bases3)
	END_CLASS_MEMBER(BaseSummer)

	BEGIN_CLASS_MEMBER(DictionaryHolder)
		CLASS_MEMBER_FIELD(maps)
		CLASS_MEMBER_FIELD(maps2)
		CLASS_MEMBER_PROPERTY_FAST(Maps)
	END_CLASS_MEMBER(DictionaryHolder)

	class TestTypeLoader : public Object, public ITypeLoader
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

namespace reflection_test
{
	int MyFunc(int a, int b)
	{
		return a + b;
	}

	void TestReflectionInvoke()
	{
		{
			Value base = Value::Create(L"Base");
			TEST_ASSERT(base.GetTypeDescriptor());
			TEST_ASSERT(base.GetTypeDescriptor()->GetTypeName() == L"Base");
			TEST_ASSERT(UnboxValue<vint>(base.GetProperty(L"a")) == 0);

			base.SetProperty(L"a", BoxValue<vint>(100));
			TEST_ASSERT(UnboxValue<vint>(base.GetProperty(L"a")) == 100);

			Value base2 = Value::Create(L"Base", (Value_xs(), BoxValue<vint>(200)));
			TEST_ASSERT(base2.GetTypeDescriptor());
			TEST_ASSERT(base2.GetTypeDescriptor()->GetTypeName() == L"Base");
			TEST_ASSERT(UnboxValue<vint>(base2.GetProperty(L"a")) == 200);

			Value base3 = Value::Create(L"Base", (Value_xs(), BoxValue<vint>(100), BoxValue<vint>(200)));
			TEST_ASSERT(base3.GetTypeDescriptor());
			TEST_ASSERT(base3.GetTypeDescriptor()->GetTypeName() == L"Base");
			TEST_ASSERT(UnboxValue<vint>(base3.GetProperty(L"a")) == 300);
		}
		{
			Value derived = Value::Create(L"Derived");
			TEST_ASSERT(derived.GetTypeDescriptor());
			TEST_ASSERT(derived.GetTypeDescriptor()->GetTypeName() == L"Derived");
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"a")) == 0);
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"b")) == 0);

			derived.SetProperty(L"a", BoxValue<vint>(100));
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"a")) == 100);
			derived.SetProperty(L"b", BoxValue<vint>(200));
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"b")) == 200);
		}
		{
			Value derived = Value::Create(L"Derived", (Value_xs(), BoxValue<vint>(10), BoxValue<vint>(20)));
			TEST_ASSERT(derived.GetTypeDescriptor());
			TEST_ASSERT(derived.GetTypeDescriptor()->GetTypeName() == L"Derived");
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"a")) == 10);
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"b")) == 20);

			derived.Invoke(L"Reset");
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"a")) == 0);
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"b")) == 0);

			derived.Invoke(L"Reset", (Value_xs(), BoxValue<vint>(30), BoxValue<vint>(40)));
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"a")) == 30);
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"b")) == 40);

			Ptr<Derived> d = UnboxValue<Ptr<Derived>>(derived);
			TEST_ASSERT(d->a == 30);
			TEST_ASSERT(d->GetB() == 40);

			Value derived2 = Value::InvokeStatic(L"Derived", L"Create", (Value_xs(), BoxValue<vint>(10), BoxValue<vint>(20)));
			derived2.Invoke(L"Reset", (Value_xs(), derived));
			TEST_ASSERT(UnboxValue<vint>(derived2.GetProperty(L"a")) == 30);
			TEST_ASSERT(UnboxValue<vint>(derived2.GetProperty(L"b")) == 40);
		}
	}

	void TestReflectionInvokeIndirect()
	{
		Value derived;
		{
			auto type = GetTypeDescriptor<Derived>();
			auto ctors = type->GetConstructorGroup();
			for (vint i = 0; i < ctors->GetMethodCount(); i++)
			{
				auto ctor = ctors->GetMethod(i);
				if (ctor->GetParameterCount() == 2)
				{
					auto proxy = ctor->CreateFunctionProxy(Value());

					auto xs = IValueList::Create();
					xs->Add(BoxValue<vint>(1));
					xs->Add(BoxValue<vint>(2));
					derived = proxy.Invoke(L"Invoke", (Value_xs(), Value::From(xs)));

					TEST_ASSERT(!derived.IsNull());
					TEST_ASSERT(UnboxValue<vint>(derived.Invoke(L"GetB", (Value_xs()))) == 2);
					break;
				}
			}
		}
		{
			auto proxy = derived.GetTypeDescriptor()->GetMethodGroupByName(L"SetB", true)->GetMethod(0)->CreateFunctionProxy(derived);
			{
				auto xs = IValueList::Create();
				xs->Add(BoxValue<vint>(3));
				proxy.Invoke(L"Invoke", (Value_xs(), Value::From(xs)));
			}
			TEST_ASSERT(UnboxValue<vint>(derived.Invoke(L"GetB", (Value_xs()))) == 3);
		}
	}

	void TestReflectionEnum()
	{
		{
			Value base = Value::Create(L"Base");
			TEST_ASSERT(UnboxValue<Season>(base.GetProperty(L"season")) == Spring);

			base.SetProperty(L"season", BoxValue<Season>(Winter));
			TEST_ASSERT(UnboxValue<Season>(base.GetProperty(L"season")) == Winter);
		}
		{
			Value derived = Value::Create(L"Derived", (Value_xs(), BoxValue<vint>(10), BoxValue<vint>(20)));
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"a")) == 10);
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"b")) == 20);

			derived.Invoke(L"Reset", (Value_xs(), BoxValue<ResetOption>(ResetNone)));
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"a")) == 10);
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"b")) == 20);
		}
		{
			Value derived = Value::Create(L"Derived", (Value_xs(), BoxValue<vint>(10), BoxValue<vint>(20)));
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"a")) == 10);
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"b")) == 20);

			derived.Invoke(L"Reset", (Value_xs(), BoxValue<ResetOption>(ResetA)));
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"a")) == 0);
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"b")) == 20);
		}
		{
			Value derived = Value::Create(L"Derived", (Value_xs(), BoxValue<vint>(10), BoxValue<vint>(20)));
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"a")) == 10);
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"b")) == 20);

			derived.Invoke(L"Reset", (Value_xs(), BoxValue<ResetOption>(ResetB)));
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"a")) == 10);
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"b")) == 0);
		}
		{
			Value derived = Value::Create(L"Derived", (Value_xs(), BoxValue<vint>(10), BoxValue<vint>(20)));
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"a")) == 10);
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"b")) == 20);

			derived.Invoke(L"Reset", (Value_xs(), BoxValue<ResetOption>((ResetOption)(ResetA | ResetB))));
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"a")) == 0);
			TEST_ASSERT(UnboxValue<vint>(derived.GetProperty(L"b")) == 0);
		}
	}

	void TestReflectionNullable()
	{
		{
			Value derived = Value::Create(L"Derived");
			TEST_ASSERT(UnboxValue<Nullable<WString>>(derived.GetProperty(L"c")) == false);
			TEST_ASSERT(UnboxValue<Nullable<WString>>(derived.GetProperty(L"C")) == false);

			derived.SetProperty(L"c", BoxValue(Nullable<WString>(L"vczh")));
			TEST_ASSERT(UnboxValue<Nullable<WString>>(derived.GetProperty(L"c")).Value() == L"vczh");
			TEST_ASSERT(UnboxValue<Nullable<WString>>(derived.GetProperty(L"C")).Value() == L"vczh");

			derived.SetProperty(L"C", BoxValue(Nullable<WString>()));
			TEST_ASSERT(UnboxValue<Nullable<WString>>(derived.GetProperty(L"c")) == false);
			TEST_ASSERT(UnboxValue<Nullable<WString>>(derived.GetProperty(L"C")) == false);
		}
	}

	void TestReflectionStruct()
	{
		{
			Point point = { 10, 20 };
			Value value = BoxValue<Point>(point);

			point = UnboxValue<Point>(value);
			TEST_ASSERT(point.x == 10);
			TEST_ASSERT(point.y == 20);
		}
		{
			Size size = { 10, 20 };
			Value value = BoxValue<Size>(size);

			size = UnboxValue<Size>(value);
			TEST_ASSERT(size.cx == 10);
			TEST_ASSERT(size.cy == 20);
		}
		{
			Rect rect = { {10, 20}, {30, 40} };
			Value value = BoxValue<Rect>(rect);

			rect = UnboxValue<Rect>(value);
			TEST_ASSERT(rect.point.x == 10);
			TEST_ASSERT(rect.point.y == 20);
			TEST_ASSERT(rect.size.cx == 30);
			TEST_ASSERT(rect.size.cy == 40);
		}
		{
			Rect a = { {1, 2}, {3, 4} };
			Rect b = { { 10, 20 }, { 30, 40 } };
			RectPair rp = { a, b };
			Value value = BoxValue<RectPair>(rp);

			rp = UnboxValue<RectPair>(value);
			TEST_ASSERT(rp.a.point.x == 1);
			TEST_ASSERT(rp.a.point.y == 2);
			TEST_ASSERT(rp.a.size.cx == 3);
			TEST_ASSERT(rp.a.size.cy == 4);
			TEST_ASSERT(rp.b.point.x == 10);
			TEST_ASSERT(rp.b.point.y == 20);
			TEST_ASSERT(rp.b.size.cx == 30);
			TEST_ASSERT(rp.b.size.cy == 40);
		}
		{
			Point point = { 10, 20 };
			Value value = BoxValue<Point>(point);
			Value a = value;
			Value b;
			b = value;

			value.SetProperty(L"x", BoxValue<vint>(1));
			value.SetProperty(L"y", BoxValue<vint>(2));

			auto pa = UnboxValue<Point>(a);
			TEST_ASSERT(pa.x == 10);
			TEST_ASSERT(pa.y == 20);

			auto pb = UnboxValue<Point>(b);
			TEST_ASSERT(pb.x == 10);
			TEST_ASSERT(pb.y == 20);
		}
	}

	void TestReflectionList()
	{
		{
			Value bases = Value::Create(L"system::Array");
			bases.Invoke(L"Resize", (Value_xs(), BoxValue<vint>(4)));
			bases.Invoke(L"Set", (Value_xs(), BoxValue<vint>(0), Value::Create(L"Base", (Value_xs(), BoxValue<vint>(1)))));
			bases.Invoke(L"Set", (Value_xs(), BoxValue<vint>(1), Value::Create(L"Base", (Value_xs(), BoxValue<vint>(2)))));
			bases.Invoke(L"Set", (Value_xs(), BoxValue<vint>(2), Value::Create(L"Base", (Value_xs(), BoxValue<vint>(3)))));
			bases.Invoke(L"Set", (Value_xs(), BoxValue<vint>(3), Value::Create(L"Base", (Value_xs(), BoxValue<vint>(4)))));

			Value baseSummer = Value::Create(L"BaseSummer");
			baseSummer.Invoke(L"SetBases", (Value_xs(), bases));
			TEST_ASSERT(UnboxValue<vint>(baseSummer.Invoke(L"Sum")) == 10);

			Value baseArray = baseSummer.Invoke(L"GetBases");
			TEST_ASSERT(UnboxValue<vint>(baseArray.GetProperty(L"Count")) == 4);
			TEST_ASSERT(UnboxValue<vint>(baseArray.Invoke(L"Get", (Value_xs(), BoxValue<vint>(0))).GetProperty(L"a")) == 1);
			TEST_ASSERT(UnboxValue<vint>(baseArray.Invoke(L"Get", (Value_xs(), BoxValue<vint>(1))).GetProperty(L"a")) == 2);
			TEST_ASSERT(UnboxValue<vint>(baseArray.Invoke(L"Get", (Value_xs(), BoxValue<vint>(2))).GetProperty(L"a")) == 3);
			TEST_ASSERT(UnboxValue<vint>(baseArray.Invoke(L"Get", (Value_xs(), BoxValue<vint>(3))).GetProperty(L"a")) == 4);
		}
		{
			Value bases = Value::Create(L"system::List");
			bases.Invoke(L"Add", (Value_xs(), Value::Create(L"Base", (Value_xs(), BoxValue<vint>(1)))));
			bases.Invoke(L"Add", (Value_xs(), Value::Create(L"Base", (Value_xs(), BoxValue<vint>(2)))));
			bases.Invoke(L"Add", (Value_xs(), Value::Create(L"Base", (Value_xs(), BoxValue<vint>(3)))));
			bases.Invoke(L"Add", (Value_xs(), Value::Create(L"Base", (Value_xs(), BoxValue<vint>(4)))));

			Value baseSummer = Value::Create(L"BaseSummer");
			baseSummer.Invoke(L"SetBases2", (Value_xs(), bases));
			TEST_ASSERT(UnboxValue<vint>(baseSummer.Invoke(L"Sum2")) == 10);

			Value baseArray = baseSummer.Invoke(L"GetBases2");
			TEST_ASSERT(UnboxValue<vint>(baseArray.GetProperty(L"Count")) == 4);
			TEST_ASSERT(UnboxValue<vint>(baseArray.Invoke(L"Get", (Value_xs(), BoxValue<vint>(0))).GetProperty(L"a")) == 1);
			TEST_ASSERT(UnboxValue<vint>(baseArray.Invoke(L"Get", (Value_xs(), BoxValue<vint>(1))).GetProperty(L"a")) == 2);
			TEST_ASSERT(UnboxValue<vint>(baseArray.Invoke(L"Get", (Value_xs(), BoxValue<vint>(2))).GetProperty(L"a")) == 3);
			TEST_ASSERT(UnboxValue<vint>(baseArray.Invoke(L"Get", (Value_xs(), BoxValue<vint>(3))).GetProperty(L"a")) == 4);
		}
		{
			Value bases = Value::Create(L"system::List");
			bases.Invoke(L"Add", (Value_xs(), Value::Create(L"Base", (Value_xs(), BoxValue<vint>(1)))));
			bases.Invoke(L"Add", (Value_xs(), Value::Create(L"Base", (Value_xs(), BoxValue<vint>(2)))));
			bases.Invoke(L"Add", (Value_xs(), Value::Create(L"Base", (Value_xs(), BoxValue<vint>(3)))));
			bases.Invoke(L"Add", (Value_xs(), Value::Create(L"Base", (Value_xs(), BoxValue<vint>(4)))));

			Value baseSummer = Value::Create(L"BaseSummer");
			TEST_EXCEPTION(baseSummer.SetProperty(L"bases3", bases), PropertyIsNotWritableException, [](auto&&) {});
		}
		{
			Value baseSummer = Value::Create(L"BaseSummer");
			Value f1 = BoxParameter(LAMBDA([](vint i) {return i + 1; }));
			Value f2 = BoxParameter(LAMBDA([](vint i) {return i + 2; }));
			Value f = baseSummer.Invoke(L"Sum3", (Value_xs(), f1, f2));
			auto fx = UnboxParameter<Func<vint(vint)>>(f);
			TEST_ASSERT(fx.Ref()(10) == 23);
		}
	}

	void TestReflectionDictionary()
	{
		Value map = Value::Create(L"system::Dictionary");
		map.Invoke(L"Set", (Value_xs(), BoxValue<vint>(1), BoxValue<vint>(1)));
		map.Invoke(L"Set", (Value_xs(), BoxValue<vint>(2), BoxValue<vint>(4)));
		map.Invoke(L"Set", (Value_xs(), BoxValue<vint>(3), BoxValue<vint>(9)));

		TEST_ASSERT(3 == UnboxValue<vint>(map.GetProperty(L"Count")));
		TEST_ASSERT(1 == UnboxValue<vint>(map.GetProperty(L"Keys").Invoke(L"Get", (Value_xs(), BoxValue<vint>(0)))));
		TEST_ASSERT(2 == UnboxValue<vint>(map.GetProperty(L"Keys").Invoke(L"Get", (Value_xs(), BoxValue<vint>(1)))));
		TEST_ASSERT(3 == UnboxValue<vint>(map.GetProperty(L"Keys").Invoke(L"Get", (Value_xs(), BoxValue<vint>(2)))));
		TEST_ASSERT(1 == UnboxValue<vint>(map.GetProperty(L"Values").Invoke(L"Get", (Value_xs(), BoxValue<vint>(0)))));
		TEST_ASSERT(4 == UnboxValue<vint>(map.GetProperty(L"Values").Invoke(L"Get", (Value_xs(), BoxValue<vint>(1)))));
		TEST_ASSERT(9 == UnboxValue<vint>(map.GetProperty(L"Values").Invoke(L"Get", (Value_xs(), BoxValue<vint>(2)))));

		map.Invoke(L"Remove", (Value_xs(), BoxValue<vint>(2)));
		TEST_ASSERT(2 == UnboxValue<vint>(map.GetProperty(L"Count")));
		TEST_ASSERT(1 == UnboxValue<vint>(map.GetProperty(L"Keys").Invoke(L"Get", (Value_xs(), BoxValue<vint>(0)))));
		TEST_ASSERT(3 == UnboxValue<vint>(map.GetProperty(L"Keys").Invoke(L"Get", (Value_xs(), BoxValue<vint>(1)))));
		TEST_ASSERT(1 == UnboxValue<vint>(map.GetProperty(L"Values").Invoke(L"Get", (Value_xs(), BoxValue<vint>(0)))));
		TEST_ASSERT(9 == UnboxValue<vint>(map.GetProperty(L"Values").Invoke(L"Get", (Value_xs(), BoxValue<vint>(1)))));

		map.Invoke(L"Clear", Value_xs());
		TEST_ASSERT(0 == UnboxValue<vint>(map.GetProperty(L"Count")));
	}

	void TestSharedRawPtrConverting()
	{
		Base* b1 = new Base;
		volatile vint* rc = ReferenceCounterOperator<Base>::CreateCounter(b1);
		TEST_ASSERT(*rc == 0);

		Ptr<Base> b2 = b1;
		TEST_ASSERT(*rc == 1);

		Value v1 = BoxValue(b1);
		TEST_ASSERT(v1.GetValueType() == Value::RawPtr);
		TEST_ASSERT(*rc == 1);

		Value v2 = BoxValue(b2);
		TEST_ASSERT(v2.GetValueType() == Value::SharedPtr);
		TEST_ASSERT(*rc == 2);

		Base* b3 = UnboxValue<Base*>(v2);
		TEST_ASSERT(b3 == b1);
		TEST_ASSERT(*rc == 2);

		Ptr<Base> b4 = UnboxValue<Ptr<Base>>(v1);
		TEST_ASSERT(b4 == b1);
		TEST_ASSERT(*rc == 3);
	}

	void TestSharedRawPtrDestructing()
	{
		{
			Ptr<Base> b = new Base;
			Ptr<Object> o = b;
			b = 0;
			o = 0;
		}
		{
			//Base* b=new Base;
			//volatile vint* rc=ReferenceCounterOperator<Base>::CreateCounter(b);
			//TEST_ASSERT(*rc==0);

			//Ptr<Object> o=b;
			//TEST_ASSERT(*rc==1);
		}
	}

	class InterfaceProxy : public Object, public IValueInterfaceProxy
	{
	public:
		IMethodInfo* lastMethodInfo = nullptr;

		Value Invoke(IMethodInfo* methodInfo, Ptr<IValueReadonlyList> arguments)
		{
			lastMethodInfo = methodInfo;
			return Value();
		}
	};

	void TestInterfaceProxy()
	{
		auto mock = MakePtr<InterfaceProxy>();
		Ptr<IValueEnumerable> proxy = ValueInterfaceProxy<IValueEnumerable>::Create(mock);
		proxy->CreateEnumerator();

		auto td = GetTypeDescriptor<IValueEnumerable>();
		auto methodInfo = td->GetMethodGroupByName(L"CreateEnumerator", false)->GetMethod(0);
		TEST_ASSERT(mock->lastMethodInfo == methodInfo);
	}

	void TestTypeInfoFriendlyName()
	{
		{
			auto typeInfo = TypeInfoRetriver<void>::CreateTypeInfo();
			TEST_ASSERT(typeInfo->GetTypeFriendlyName() == L"system::Void");
		}
		{
			auto typeInfo = TypeInfoRetriver<Nullable<vint32_t>>::CreateTypeInfo();
			TEST_ASSERT(typeInfo->GetTypeFriendlyName() == L"system::Int32?");
		}
		{
			auto typeInfo = TypeInfoRetriver<Base*>::CreateTypeInfo();
			TEST_ASSERT(typeInfo->GetTypeFriendlyName() == L"Base*");
		}
		{
			auto typeInfo = TypeInfoRetriver<Ptr<Base>>::CreateTypeInfo();
			TEST_ASSERT(typeInfo->GetTypeFriendlyName() == L"Base^");
		}
		{
			auto typeInfo = TypeInfoRetriver<List<Ptr<Base>>>::CreateTypeInfo();
			TEST_ASSERT(typeInfo->GetTypeFriendlyName() == L"system::List<Base^>^");
		}
		{
			auto typeInfo = TypeInfoRetriver<Func<void(vint32_t)>>::CreateTypeInfo();
			TEST_ASSERT(typeInfo->GetTypeFriendlyName() == L"system::Function<system::Void, system::Int32>^");
		}
	}

	void TestCpp()
	{
		{
			auto td = GetTypeDescriptor<void>();
			TEST_ASSERT(CppExists(td));
			TEST_ASSERT(CppGetFullName(td) == L"void");
		}
		{
			auto td = GetTypeDescriptor<vint32_t>();
			TEST_ASSERT(CppExists(td));
			TEST_ASSERT(CppGetFullName(td) == L"::vl::vint32_t");
		}
		{
			auto td = GetTypeDescriptor<IValueEnumerable>();
			TEST_ASSERT(CppExists(td));
			TEST_ASSERT(CppGetFullName(td) == L"::vl::reflection::description::IValueEnumerable");
		}
		{
			auto td = GetTypeDescriptor<Base>();
			TEST_ASSERT(CppExists(td));
			TEST_ASSERT(CppGetFullName(td) == L"::Base");
		}

		{
			auto td = GetTypeDescriptor<Point>();
			auto prop = td->GetPropertyByName(L"x", false);
			TEST_ASSERT(CppExists(prop));
			TEST_ASSERT(CppGetReferenceTemplate(prop) == L"$This.$Name");
		}
		{
			auto td = GetTypeDescriptor<Base>();
			auto prop = td->GetPropertyByName(L"a", false);
			TEST_ASSERT(CppExists(prop));
			TEST_ASSERT(CppGetReferenceTemplate(prop) == L"$This->$Name");
		}
		{
			auto td = GetTypeDescriptor<Derived>();
			auto prop = td->GetPropertyByName(L"b", false);
			TEST_ASSERT(CppExists(prop));
		}

		{
			auto td = GetTypeDescriptor<Base>();
			{
				auto ctor = td->GetConstructorGroup()->GetMethod(0);
				TEST_ASSERT(ctor->GetParameterCount() == 0);
				TEST_ASSERT(CppExists(ctor));
				TEST_ASSERT(CppGetInvokeTemplate(ctor) == L"new $Type($Arguments)");
			}
			{
				auto ctor = td->GetConstructorGroup()->GetMethod(1);
				TEST_ASSERT(ctor->GetParameterCount() == 1);
				TEST_ASSERT(CppExists(ctor));
				TEST_ASSERT(CppGetInvokeTemplate(ctor) == L"new $Type($Arguments)");
			}
			{
				auto ctor = td->GetConstructorGroup()->GetMethod(2);
				TEST_ASSERT(ctor->GetParameterCount() == 2);
				TEST_ASSERT(CppExists(ctor));
				TEST_ASSERT(CppGetInvokeTemplate(ctor) == L"::Base::Create($Arguments)");
			}
		}

		{
			auto td = GetTypeDescriptor<Derived>();
			{
				auto method = td->GetMethodGroupByName(L"Create", false)->GetMethod(0);
				TEST_ASSERT(method->GetParameterCount() == 0);
				TEST_ASSERT(CppExists(method));
				TEST_ASSERT(CppGetInvokeTemplate(method) == L"$Type::$Name($Arguments)");
			}
			{
				auto method = td->GetMethodGroupByName(L"Reset", false)->GetMethod(0);
				TEST_ASSERT(method->GetParameterCount() == 0);
				TEST_ASSERT(CppExists(method));
				TEST_ASSERT(CppGetInvokeTemplate(method) == L"$This->$Name($Arguments)");
			}
			{
				auto method = td->GetMethodGroupByName(L"Reset", false)->GetMethod(1);
				TEST_ASSERT(method->GetParameterCount() == 2);
				TEST_ASSERT(CppExists(method));
				TEST_ASSERT(CppGetInvokeTemplate(method) == L"$This->$Name($Arguments)");
			}
			{
				auto method = td->GetMethodGroupByName(L"Reset", false)->GetMethod(2);
				TEST_ASSERT(method->GetParameterCount() == 1);
				TEST_ASSERT(CppExists(method));
				TEST_ASSERT(CppGetInvokeTemplate(method) == L"$This->Reset2($Arguments)");
			}
			{
				auto method = td->GetMethodGroupByName(L"Reset", false)->GetMethod(3);
				TEST_ASSERT(method->GetParameterCount() == 1);
				TEST_ASSERT(CppExists(method));
				TEST_ASSERT(CppGetInvokeTemplate(method) == L"::Derived::Reset3($This, $Arguments)");
			}
		}
		{
			auto td = GetTypeDescriptor<ITypeDescriptor>();
			auto method = td->GetMethodGroupByName(L"GetTypeDescriptorCount", false)->GetMethod(0);
			TEST_ASSERT(CppExists(method));
			TEST_ASSERT(CppGetInvokeTemplate(method) == L"::vl::reflection::description::ITypeDescriptor_GetTypeDescriptorCount($Arguments)");
		}
	}
}
using namespace reflection_test;

#define TEST_CASE_REFLECTION(NAME)\
	TEST_CASE(L ## #NAME)\
	{\
		TEST_ASSERT(LoadPredefinedTypes());\
		TEST_ASSERT(GetGlobalTypeManager()->AddTypeLoader(new TestTypeLoader));\
		TEST_ASSERT(GetGlobalTypeManager()->Load());\
		{\
			NAME();\
		}\
		TEST_ASSERT(ResetGlobalTypeManager());\
	});\

TEST_FILE
{
	TEST_CASE_REFLECTION(TestReflectionInvoke)
	TEST_CASE_REFLECTION(TestReflectionInvokeIndirect)
	TEST_CASE_REFLECTION(TestReflectionEnum)
	TEST_CASE_REFLECTION(TestReflectionNullable)
	TEST_CASE_REFLECTION(TestReflectionStruct)
	TEST_CASE_REFLECTION(TestReflectionList)
	TEST_CASE_REFLECTION(TestReflectionDictionary)
	TEST_CASE_REFLECTION(TestSharedRawPtrConverting)
	TEST_CASE_REFLECTION(TestSharedRawPtrDestructing)
	TEST_CASE_REFLECTION(TestInterfaceProxy)
	TEST_CASE_REFLECTION(TestTypeInfoFriendlyName)
	TEST_CASE_REFLECTION(TestCpp)
}
