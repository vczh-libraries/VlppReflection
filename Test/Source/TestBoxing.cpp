#include "Common.h"
#include <limits>

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection;
using namespace vl::reflection::description;

#ifdef VCZH_DEBUG_METAONLY_REFLECTION
extern bool LoadPredefinedTypesForTestCase();

#define TEST_CASE_VARIANT(NAME)\
	TEST_CASE(L ## #NAME)\
	{\
		TEST_ASSERT(LoadPredefinedTypesForTestCase());\
		NAME();\
		TEST_ASSERT(ResetGlobalTypeManager());\
	});
#else
#define TEST_CASE_VARIANT(NAME) TEST_CASE_REFLECTION_NOLOADER(NAME)
#endif

namespace TestBoxing_TestObjects_Boxing
{
	class Base : public Object, public Description<Base>
	{
	};

	class Derived : public Base, public Description<Derived>
	{
	};

	using VariantTestValue = Variant<vint, WString>;

	struct VariantStruct
	{
		VariantTestValue value;

		std::partial_ordering operator<=>(const VariantStruct& other)const
		{
			return BoxParameter(value) <=> BoxParameter(other.value);
		}
	};

	class VariantHolder : public Object, public Description<VariantHolder>
	{
	public:
		VariantTestValue value;
		Event<void(VariantTestValue)> Changed;

		VariantTestValue Echo(VariantTestValue input) { return input; }
		VariantTestValue EchoConst(const VariantTestValue& input) { return input; }
		VariantTestValue EchoRef(VariantTestValue& input) { return input; }
		VariantTestValue GetProperty() { return value; }
		void SetProperty(VariantTestValue input) { value = input; }
	};
}
using namespace TestBoxing_TestObjects_Boxing;

#ifndef VCZH_DEBUG_NO_REFLECTION

#define _ ,

#define TYPE_LIST(F)\
	F(Base)\
	F(Derived)\
	F(VariantStruct)\
	F(VariantHolder)\

namespace vl::reflection::description
{

	TYPE_LIST(DECL_TYPE_INFO)
	TYPE_LIST(IMPL_CPP_TYPE_INFO)

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
	BEGIN_STRUCT_MEMBER(VariantStruct)
		STRUCT_MEMBER(value)
	END_STRUCT_MEMBER(VariantStruct)

	BEGIN_CLASS_MEMBER(VariantHolder)
		CLASS_MEMBER_CONSTRUCTOR(Ptr<VariantHolder>(), NO_PARAMETER)
		CLASS_MEMBER_FIELD(value)
		CLASS_MEMBER_METHOD(Echo, { L"input" })
		CLASS_MEMBER_METHOD(EchoConst, { L"input" })
		CLASS_MEMBER_METHOD(EchoRef, { L"input" })
		CLASS_MEMBER_PROPERTY_FAST(Property)
		CLASS_MEMBER_EVENT(Changed)
	END_CLASS_MEMBER(VariantHolder)

	class VariantTestTypeLoader : public Object, public ITypeLoader
	{
	public:
		void Load(ITypeManager* manager)override
		{
			ADD_TYPE_INFO(VariantStruct)
			ADD_TYPE_INFO(VariantHolder)
		}

		void Unload(ITypeManager* manager)override
		{
		}
	};
#endif

}

#undef TYPE_LIST

#endif

void TestVariantParameters()
{
	using T = Variant<vint, WString, Nullable<vint>, IDescriptable*, Ptr<IDescriptable>, Value>;
	auto check = [](const T& source)
	{
		auto box = BoxParameter(source);
		TEST_ASSERT(!box.IsNull());
		TEST_ASSERT(box.GetValueType() == Value::BoxedValue);
		auto copied = Value::From(box.GetBoxedValue()->Copy(), nullptr);
		auto first = UnboxParameter<T>(box);
		auto second = UnboxParameter<T>(copied);
		TEST_ASSERT(first.IsOwned());
		TEST_ASSERT(first.Ref().Index() == source.Index());
		TEST_ASSERT(second.Ref().Index() == source.Index());
		auto arguments = IValueList::Create();
		arguments->Add(box);
		Func<T(T)> byValue = [](T input) { return input; };
		Func<T(const T&)> byConst = [](const T& input) { return input; };
		Func<T(T&)> byRef = [](T& input) { return input; };
		for (auto callable : { BoxParameter(byValue), BoxParameter(byConst), BoxParameter(byRef) })
		{
			auto proxy = UnboxValue<Ptr<IValueFunctionProxy>>(callable);
			auto result = UnboxParameter<T>(proxy->Invoke(arguments));
			TEST_ASSERT(result.Ref().Index() == source.Index());
			source.Apply([&]<typename TOption>(const TOption& option)
			{
				TEST_ASSERT(result.Ref().Get<TOption>() == option);
			});
		}
		source.Apply([&]<typename TOption>(const TOption& option)
		{
			TEST_ASSERT(first.Ref().Get<TOption>() == option);
			TEST_ASSERT(second.Ref().Get<TOption>() == option);
#ifndef VCZH_DEBUG_NO_REFLECTION
			auto dynamicBox = box.GetBoxedValue().Cast<IValueType::TypedBox<DynamicVariantBox>>();
			TEST_ASSERT(dynamicBox);
			auto type = TypeInfoRetriver<TOption>::CreateTypeInfo();
			TEST_ASSERT(IsSameTypeInfo(dynamicBox->value.type.Obj(), type.Obj()));
#else
			TEST_ASSERT(box.GetBoxedValue().Cast<IValueType::TypedBox<StaticVariantBox<TOption>>>());
#endif
		});
	};
	check(T(vint(42)));
	check(T(WString(L"variant")));
	check(T(Nullable<vint>(42)));
	check(T(Nullable<vint>()));
	check(T(static_cast<IDescriptable*>(nullptr)));
	check(T(Ptr<IDescriptable>()));
	check(T(Value()));
	auto instance = IValueList::Create();
	check(T(static_cast<IDescriptable*>(instance.Obj())));
	check(T(Ptr<IDescriptable>(instance)));

	T source = WString(L"saved");
	auto saved = BoxParameter(source);
	source = vint(1);
	TEST_ASSERT(UnboxParameter<T>(saved).Ref().Get<WString>() == L"saved");

	List<T> values;
	auto list = UnboxValue<Ptr<IValueList>>(BoxParameter(values));
	list->Add(saved);
	TEST_ASSERT(values.Count() == 1);
	TEST_ASSERT(values[0].Get<WString>() == L"saved");
	TEST_ASSERT(UnboxParameter<T>(list->Get(0)).Ref().Get<WString>() == L"saved");
	TEST_ASSERT(list->Contains(saved));
	TEST_ASSERT(list->IndexOf(saved) == 0);
	auto lazy = GetLazyList<T>(list);
	TEST_ASSERT(lazy.First().Get<WString>() == L"saved");
	auto lazyBox = BoxParameter(lazy);
	TEST_ASSERT(UnboxParameter<LazyList<T>>(lazyBox).Ref().First().Get<WString>() == L"saved");
	TEST_ASSERT(list->Remove(saved));
	TEST_ASSERT(values.Count() == 0);
}

void TestVariantCollectionAndFunction()
{
	using T = Variant<vint, List<vint>, Func<vint(vint)>>;
	Value saved;
	{
		T source(VariantIndex<1>{});
		source.Get<List<vint>>().Add(10);
		saved = BoxParameter(source);
		source = vint(0);
	}
	auto copied = Value::From(saved.GetBoxedValue()->Copy(), nullptr);
	auto first = UnboxParameter<T>(saved);
	auto second = UnboxParameter<T>(copied);
	TEST_ASSERT(first.Ref().Index() == 1);
	TEST_ASSERT(first.Ref().Get<List<vint>>().Count() == 1);
	TEST_ASSERT(first.Ref().Get<List<vint>>()[0] == 10);
	first.Ref().Get<List<vint>>().Set(0, 20);
	TEST_ASSERT(second.Ref().Get<List<vint>>()[0] == 10);

	T function = Func<vint(vint)>([](vint value) { return value + 1; });
	auto functionBox = BoxParameter(function);
	TEST_ASSERT(UnboxParameter<T>(functionBox).Ref().Get<Func<vint(vint)>>()(10) == 11);

	Func<T(T)> echo = [](T input) { return input; };
	auto proxy = UnboxValue<Ptr<IValueFunctionProxy>>(BoxParameter(echo));
	auto arguments = IValueList::Create();
	arguments->Add(saved);
	auto echoed = UnboxParameter<T>(proxy->Invoke(arguments));
	TEST_ASSERT(echoed.Ref().Get<List<vint>>()[0] == 10);
	echoed.Ref().Get<List<vint>>().Set(0, 30);
	TEST_ASSERT(UnboxParameter<T>(saved).Ref().Get<List<vint>>()[0] == 10);

	Func<T(const T&)> echoConst = [](const T& input)
	{
		List<vint> copy;
		CopyFrom(copy, input.Get<List<vint>>());
		return T(std::move(copy));
	};
	proxy = UnboxValue<Ptr<IValueFunctionProxy>>(BoxParameter(echoConst));
	TEST_ASSERT(UnboxParameter<T>(proxy->Invoke(arguments)).Ref().Get<List<vint>>()[0] == 10);
	Func<T(T&)> echoRef = [](T& input) { return std::move(input); };
	proxy = UnboxValue<Ptr<IValueFunctionProxy>>(BoxParameter(echoRef));
	TEST_ASSERT(UnboxParameter<T>(proxy->Invoke(arguments)).Ref().Get<List<vint>>()[0] == 10);
	TEST_ASSERT(UnboxParameter<T>(saved).Ref().Get<List<vint>>()[0] == 10);
}

void TestVariantCollectionOptions()
{
	auto check = []<typename TCollection>(TCollection&& source)
	{
		using T = Variant<vint, TCollection>;
		T input(VariantIndex<1>{});
		if constexpr (std::is_copy_assignable_v<TCollection>)
		{
			input.template Get<TCollection>() = source;
		}
		else
		{
			CopyFrom(input.template Get<TCollection>(), source);
		}
		auto box = BoxParameter(input);
		input = vint(0);
		auto copied = Value::From(box.GetBoxedValue()->Copy(), nullptr);
		auto first = UnboxParameter<T>(box);
		auto second = UnboxParameter<T>(copied);
		TEST_ASSERT(first.Ref().Index() == 1);
		TEST_ASSERT(second.Ref().Index() == 1);
		TEST_ASSERT(From(first.Ref().template Get<TCollection>()).First() == From(second.Ref().template Get<TCollection>()).First());
		auto arguments = IValueList::Create();
		arguments->Add(box);
		Func<vint(const T&)> count = [](const T& input) { return From(input.template Get<TCollection>()).Count(); };
		auto proxy = UnboxValue<Ptr<IValueFunctionProxy>>(BoxParameter(count));
		TEST_ASSERT(UnboxValue<vint>(proxy->Invoke(arguments)) == 1);
		if constexpr (std::is_move_constructible_v<TCollection>)
		{
			Func<T(T)> echo = [](T input) { return input; };
			proxy = UnboxValue<Ptr<IValueFunctionProxy>>(BoxParameter(echo));
			auto result = UnboxParameter<T>(proxy->Invoke(arguments));
			TEST_ASSERT(From(result.Ref().template Get<TCollection>()).First() == From(second.Ref().template Get<TCollection>()).First());
		}
	};
	Array<vint> array(1);
	array.Set(0, 10);
	check(std::move(array));
	SortedList<vint> sorted;
	sorted.Add(10);
	check(std::move(sorted));
	ObservableList<vint> observable;
	observable.Add(10);
	check(std::move(observable));
	Dictionary<vint, WString> dictionary;
	dictionary.Add(10, L"ten");
	check(std::move(dictionary));
	check(Range<vint>(10, 1));
}

void TestVariantComparison()
{
	using T = Variant<double, WString>;
	auto first = BoxParameter(T(1.0));
	auto equal = BoxParameter(T(1.0));
	auto greater = BoxParameter(T(2.0));
	TEST_ASSERT(first == equal);
	TEST_ASSERT(first < greater);
	TEST_ASSERT(greater > first);
	TEST_ASSERT(first <= equal);
	TEST_ASSERT(first >= equal);
	TEST_ASSERT(first != greater);
	auto unordered = BoxParameter(T(std::numeric_limits<double>::quiet_NaN()));
	TEST_ASSERT((unordered <=> first) == std::partial_ordering::unordered);
	TEST_ASSERT(unordered != first);
	TEST_ASSERT(!(unordered == first));
	TEST_ASSERT(!(unordered < first));
	TEST_ASSERT(!(unordered <= first));
	TEST_ASSERT(!(unordered > first));
	TEST_ASSERT(!(unordered >= first));

	auto integer = BoxValue<vint>(1);
	auto floating = BoxValue<double>(1.0);
	TEST_ASSERT(CompareVariantPayloads(integer, floating) == std::partial_ordering::equivalent);
	TEST_ASSERT(CompareVariantPayloads(BoxValue<vint>(-1), BoxValue<vuint64_t>(0)) == std::partial_ordering::less);
	TEST_ASSERT(CompareVariantPayloads(BoxValue<vuint64_t>(_UI64_MAX), integer) == std::partial_ordering::greater);
	TEST_ASSERT(CompareVariantPayloads(integer, BoxValue<double>(std::numeric_limits<double>::quiet_NaN())) == std::partial_ordering::unordered);
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
	TEST_ASSERT(integer == floating);
#else
	TEST_ASSERT((integer <=> floating) == std::partial_ordering::unordered);
#endif

	using P = Variant<IValueList*, vint>;
	auto instance = IValueList::Create();
	auto pointer = BoxParameter(P(instance.Obj()));
	auto samePointer = BoxParameter(P(instance.Obj()));
	TEST_ASSERT(pointer == samePointer);
	auto nullPointer = BoxParameter(P(static_cast<IValueList*>(nullptr)));
	TEST_ASSERT(!nullPointer.IsNull());
	TEST_ASSERT(pointer != nullPointer);
}

void TestVariantFailures()
{
	auto expectMismatch = [](auto&& action)
	{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
		TEST_EXCEPTION(action(), ArgumentTypeMismtatchException, [](auto&&) {});
#else
		TEST_EXCEPTION(action(), Error, [](auto&&) {});
#endif
	};
	using T = Variant<IDescriptable*, IValueList*, Ptr<IDescriptable>, Ptr<IValueList>, Nullable<vint>, Value>;
	using U = Variant<IValueList*, vint>;
	auto list = IValueList::Create();
	T source = static_cast<IDescriptable*>(list.Obj());
	auto box = BoxParameter(source);
	TEST_ASSERT(UnboxParameter<T>(box).Ref().Index() == 0);
	expectMismatch([&]() { UnboxParameter<U>(box); });
	expectMismatch([&]() { UnboxParameter<T>(BoxValue<vint>(1)); });
	expectMismatch([&]() { UnboxValue<vint>(box); });
	source = static_cast<IDescriptable*>(nullptr);
	expectMismatch([&]() { UnboxParameter<U>(BoxParameter(source)); });
	source = static_cast<IValueList*>(nullptr);
	TEST_ASSERT(UnboxParameter<U>(BoxParameter(source)).Ref().Get<IValueList*>() == nullptr);
	auto reordered = UnboxParameter<Variant<Value, Nullable<vint>, Ptr<IValueList>, Ptr<IDescriptable>, IValueList*, IDescriptable*>>(BoxParameter(source));
	TEST_ASSERT(reordered.Ref().Index() == 4);
}

#ifndef VCZH_DEBUG_NO_REFLECTION
void TestVariantTypeInfo()
{
	using T = Variant<vint, WString, Nullable<vint>>;
	auto first = TypeInfoRetriver<T>::CreateTypeInfo();
	auto second = TypeInfoRetriver<const T&>::CreateTypeInfo();
	auto reordered = TypeInfoRetriver<Variant<WString, vint, Nullable<vint>>>::CreateTypeInfo();
	TEST_ASSERT(first->GetDecorator() == ITypeInfo::Variant);
	TEST_ASSERT(first->GetHint() == TypeInfoHint::Normal);
	TEST_ASSERT(!first->GetTypeDescriptor());
	TEST_ASSERT(!first->GetElementType());
	TEST_ASSERT(first->GetGenericArgumentCount() == 3);
	TEST_ASSERT(first->GetGenericArgument(0)->GetTypeDescriptor() == GetTypeDescriptor<vint>());
	TEST_ASSERT(first->GetGenericArgument(1)->GetTypeDescriptor() == GetTypeDescriptor<WString>());
	TEST_ASSERT(first->GetGenericArgument(2)->GetDecorator() == ITypeInfo::Nullable);
	TEST_ASSERT(IsSameTypeInfo(first.Obj(), second.Obj()));
	TEST_ASSERT(!IsSameTypeInfo(first.Obj(), reordered.Obj()));
	TEST_ASSERT(first->GetTypeFriendlyName() == L"Variant<" + GetTypeDescriptor<vint>()->GetTypeName() + L", " + GetTypeDescriptor<WString>()->GetTypeName() + L", " + GetTypeDescriptor<vint>()->GetTypeName() + L"?>");
}
#endif

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
void TestVariantMembers()
{
	auto holder = Ptr(new VariantHolder);
	auto object = BoxValue(holder);
	for (auto input : { BoxParameter(VariantTestValue(vint(42))), BoxParameter(VariantTestValue(WString(L"field"))) })
	{
		object.SetProperty(L"value", input);
		TEST_ASSERT(object.GetProperty(L"value") == input);
		object.SetProperty(L"Property", input);
		TEST_ASSERT(object.GetProperty(L"Property") == input);
		for (auto name : { L"Echo", L"EchoConst", L"EchoRef" })
		{
			TEST_ASSERT(object.Invoke(name, (Value_xs(), input)) == input);
		}
		auto structObject = BoxValue(VariantStruct{});
		structObject.SetProperty(L"value", input);
		TEST_ASSERT(structObject.GetProperty(L"value") == input);
	}
	auto copy = UnboxParameter<VariantTestValue>(object.GetProperty(L"value"));
	copy.Ref() = vint(10);
	TEST_ASSERT(holder->value.Get<WString>() == L"field");
	TEST_EXCEPTION(object.SetProperty(L"value", BoxValue<vint>(1)), ArgumentTypeMismtatchException, [](auto&&) {});

	auto called = false;
	auto handler = holder->Changed.Add([&](VariantTestValue input)
	{
		called = true;
		TEST_ASSERT(input.Get<WString>() == L"event");
	});
	auto eventInfo = GetTypeDescriptor<VariantHolder>()->GetEventByName(L"Changed", false);
	auto arguments = IValueList::Create();
	arguments->Add(BoxParameter(VariantTestValue(WString(L"event"))));
	eventInfo->Invoke(object, arguments);
	TEST_ASSERT(called);
	holder->Changed.Remove(handler);
}

void TestVariantMetadata()
{
	stream::MemoryStream metadata;
	{
		List<ITypeDescriptor*> exclusions;
		GenerateMetaonlyTypes(exclusions, metadata);
	}
	TEST_ASSERT(ResetGlobalTypeManager());
	metadata.SeekFromBegin(0);
	Dictionary<WString, Ptr<ISerializableType>> serializers;
#define INSTALL_VARIANT_SERIALIZER(TYPE) serializers.Add(TypeInfo<TYPE>::content.typeName, Ptr(new SerializableType<TYPE>()));
	REFLECTION_PREDEFINED_SERIALIZABLE_TYPES(INSTALL_VARIANT_SERIALIZER)
#undef INSTALL_VARIANT_SERIALIZER
	auto loader = LoadMetaonlyTypes(metadata, serializers);
	auto manager = GetGlobalTypeManager();
	TEST_ASSERT(manager->AddTypeLoader(loader));
	TEST_ASSERT(manager->Load());
	auto holder = GetTypeDescriptor<VariantHolder>();
	auto field = holder->GetPropertyByName(L"value", false)->GetReturn();
	auto expected = TypeInfoRetriver<VariantTestValue>::CreateTypeInfo();
	TEST_ASSERT(field->GetDecorator() == ITypeInfo::Variant);
	TEST_ASSERT(field->GetHint() == TypeInfoHint::Normal);
	TEST_ASSERT(!field->GetTypeDescriptor());
	TEST_ASSERT(!field->GetElementType());
	TEST_ASSERT(IsSameTypeInfo(field, expected.Obj()));
	TEST_ASSERT(field->GetTypeFriendlyName() == expected->GetTypeFriendlyName());
	auto structure = GetTypeDescriptor<VariantStruct>()->GetPropertyByName(L"value", false)->GetReturn();
	TEST_ASSERT(IsSameTypeInfo(structure, expected.Obj()));
}
#endif

TEST_FILE
{
#ifndef VCZH_DEBUG_NO_REFLECTION
	TEST_CASE_VARIANT(TestVariantParameters)
	TEST_CASE_VARIANT(TestVariantCollectionAndFunction)
	TEST_CASE_VARIANT(TestVariantCollectionOptions)
	TEST_CASE_VARIANT(TestVariantComparison)
	TEST_CASE_VARIANT(TestVariantTypeInfo)
	TEST_CASE_VARIANT(TestVariantFailures)
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
	TEST_CASE_REFLECTION_LOADER(TestVariantMembers, VariantTestTypeLoader)
	TEST_CASE_REFLECTION_LOADER(TestVariantMetadata, VariantTestTypeLoader)
#endif
#else
	TEST_CASE(L"TestVariantParameters")
	{
		TestVariantParameters();
	});
	TEST_CASE(L"TestVariantCollectionAndFunction")
	{
		TestVariantCollectionAndFunction();
	});
	TEST_CASE(L"TestVariantComparison")
	{
		TestVariantComparison();
	});
	TEST_CASE(L"TestVariantCollectionOptions")
	{
		TestVariantCollectionOptions();
	});
	TEST_CASE(L"TestVariantFailures")
	{
		TestVariantFailures();
	});
#endif
	TEST_CASE(L"Test variant parameter classification")
	{
		using T = Variant<vint, WString>;
		TEST_ASSERT(TypeInfoRetriver<T>::TypeFlag != TypeFlags::NonGenericType);
		TEST_ASSERT(TypeInfoRetriver<const T&>::TypeFlag == TypeInfoRetriver<T>::TypeFlag);
		TEST_ASSERT(TypeInfoRetriver<T&>::Hint == TypeInfoHint::Normal);
	});

	TEST_CASE(L"Test DescriptableObject: ReferenceCounterOperator")
	{
		TEST_ASSERT((!std::is_convertible_v<vint*, DescriptableObject*>));
		TEST_ASSERT((std::is_convertible_v<DescriptableObject*, DescriptableObject*>));
		TEST_ASSERT((std::is_convertible_v<IDescriptable*, DescriptableObject*>));
		TEST_ASSERT((std::is_convertible_v<Base*, DescriptableObject*>));
		TEST_ASSERT((std::is_convertible_v<Derived*, DescriptableObject*>));

		Base* raw = new Base;
		atomic_vint* counter = ReferenceCounterOperator<Base>::CreateCounter(raw);
		TEST_ASSERT(0 == *counter);
		{
			Ptr<Base> ptr1(raw);
			TEST_ASSERT(1 == *counter);
			{
				Ptr<Base> ptr2(raw);
				TEST_ASSERT(2 == *counter);
				{
					Ptr<Base> ptr3(raw);
					TEST_ASSERT(3 == *counter);
				}
				TEST_ASSERT(2 == *counter);
			}
			TEST_ASSERT(1 == *counter);
		}
	});

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
	TEST_CASE(L"Test DescriptableObject: Boxing and unboxing")
	{
		TEST_ASSERT(LoadPredefinedTypes());
		TEST_ASSERT(GetGlobalTypeManager()->Load());
		{
			auto value = BoxValue<vint>(0);
			TEST_ASSERT(UnboxValue<vint>(value) == 0);
		}
		{
			auto value = BoxValue<WString>(L"abc");
			TEST_ASSERT(UnboxValue<WString>(value) == L"abc");
		}
		{
			auto base = Ptr(new Base);
			{
				auto value = BoxValue<Base*>(base.Obj());
				TEST_ASSERT(UnboxValue<Base*>(value) == base.Obj());
				TEST_ASSERT(UnboxValue<Ptr<Base>>(value) == base);
			}
			{
				auto value = BoxValue<Ptr<Base>>(base);
				TEST_ASSERT(UnboxValue<Base*>(value) == base.Obj());
				TEST_ASSERT(UnboxValue<Ptr<Base>>(value) == base);
			}
		}
		{
			List<vint> numbers;
			numbers.Add(1);
			numbers.Add(2);
			numbers.Add(3);

			auto value = BoxParameter(numbers);
			auto numbers2 = UnboxParameter<List<vint>>(value);

			TEST_ASSERT(numbers2.Ref().Count() == 3);
			TEST_ASSERT(numbers2.Ref()[0] == 1);
			TEST_ASSERT(numbers2.Ref()[1] == 2);
			TEST_ASSERT(numbers2.Ref()[2] == 3);
		}
		TEST_ASSERT(ResetGlobalTypeManager());
	});

#endif
}
