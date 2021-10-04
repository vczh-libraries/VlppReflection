/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_GUITYPEDESCRIPTOR
#define VCZH_REFLECTION_GUITYPEDESCRIPTOR

#include <VlppOS.h>

#if (defined VCZH_DEBUG_NO_REFLECTION) && (defined VCZH_DEBUG_METAONLY_REFLECTION)
static_assert(false, "Preprocessor VCZH_DEBUG_NO_REFLECTION and VCZH_DEBUG_METAONLY_REFLECTION could not be defined at the same time.")
#endif

#if !(defined VCZH_DEBUG_NO_REFLECTION) && !(defined VCZH_DEBUG_METAONLY_REFLECTION)
#define VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
#endif

namespace vl
{
	namespace reflection
	{

/***********************************************************************
Attribute
***********************************************************************/

		namespace description
		{
			class ITypeDescriptor;
			class ITypeInfo;
			class IEventHandler;
			class IEventInfo;
			class IPropertyInfo;
			class IParameterInfo;
			class IMethodInfo;
			class IMethodGroupInfo;

			class IValueFunctionProxy;
			class IValueInterfaceProxy;
			class IValueSubscription;

			class IValueEnumerable;
			class IValueEnumerator;
			class IValueReadonlyList;
			class IValueList;
			class IValueObservableList;
			class IValueReadonlyDictionary;
			class IValueDictionary;

			class IValueCallStack;
			class IValueException;

			template<typename T>
			struct TypedValueSerializerProvider
			{
			};
		}

		/// <summary>
		/// <p>
		/// Base class of all reflectable value types (class).
		/// If you want to create a reflectable class, you should inherit from [T:vl.reflection.Description`1].
		/// </p>
		/// <p>
		/// Inheriting from [T:vl.reflection.Description`1] is necessary even if you turned on "VCZH_DEBUG_NO_REFLECTION" preprocessor definition.
		/// In this case, some members will be removed from this class to reduce the object size.
		/// </p>
		/// <p>
		/// <b>Ptr&lt;DescriptableObject&gt;</b> is recommended to replace <b>Ptr&lt;Object&gt;</b> for holding a reflectable object.
		/// When a class <b>T</b> inherits from [T:vl.reflection.Description`1], including <b>DescriptableObject</b> itself,
		/// <b>Ptr&lt;T&gt;</b> is safe to be created directly from a <b>T*</b> hold by another <b>Ptr&lt;T&gt;</b>.
		/// This is not allowed for all classes that do not inherit from [T:vl.reflection.Description`1].
		/// </p>
		/// </summary>
		/// <remarks>
		/// <p>
		/// When a class in Workflow script inherits from a class in C++,
		/// since it is not possible to actually create a class in runtime,
		/// so the created object from this Workflow class is multiple <b>DescriptableObject</b> grouping together.
		/// </p>
		/// <p>
		/// This is called <b>aggregation</b>.
		/// </p>
		/// <p>
		/// In this case, <see cref="SafeAggregationCast`1"/> is required to do pointer casting to a C++ class.
		/// </p>
		/// <p>
		/// To allow a C++ class to be aggregated,
		/// use [T:vl.reflection.AggregatableDescription`1] instead of [T:vl.reflection.Description`1],
		/// and call <see cref="FinalizeAggregation"/> in the destructor.
		/// If A inherits B and they are all aggregatable, do it in both destructors.
		/// </p>
		/// </remarks>
		/// <example><![CDATA[
		/// class MyClass : public Object, public Description<MyClass>
		/// {
		/// public:
		///     WString data;
		/// };
		/// 
		/// int main()
		/// {
		///     auto myClass = MakePtr<MyClass>();
		///     myClass->data = L"Hello, world!";
		/// 
		///     Ptr<DescriptableObject> obj = myClass;
		///     Console::WriteLine(obj.Cast<MyClass>()->data);
		/// 
		///     // usually you cannot do this directly
		///     // because obj and myClass share the same reference counter, but myClass2 doesn't
		///     // this will cause the destructor delete MyClass twice and crash
		///     // but it is different when MyClass inherits from Description<MyClass> or AggregatableDescription<MyClass>
		///     auto myClass2 = Ptr<MyClass>(dynamic_cast<MyClass*>(obj.Obj()));
		///     Console::WriteLine(myClass2->data);
		/// }
		/// ]]></example>
		class DescriptableObject
		{
			template<typename T, typename Enabled>
			friend struct vl::ReferenceCounterOperator;
			template<typename T>
			friend class Description;

			typedef collections::Dictionary<WString, Ptr<Object>>		InternalPropertyMap;
			typedef bool(*DestructorProc)(DescriptableObject* obj, bool forceDisposing);
		private:
			volatile vint							referenceCounter;

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			size_t									objectSize;
			description::ITypeDescriptor**			typeDescriptor;
#endif
			Ptr<InternalPropertyMap>				internalProperties;


#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			bool									destructing;
			DescriptableObject**					aggregationInfo;
			vint									aggregationSize;
#endif

		protected:
			DestructorProc							sharedPtrDestructorProc;

		protected:

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			// Returns true if this object inherits other objects by aggregation.</returns>
			bool									IsAggregated();

			// Returnd the number of aggregated base objects.</returns>
			vint									GetAggregationSize();

			// Return the object that inherit this object.</returns>
			DescriptableObject*						GetAggregationRoot();

			// Notice that an object inherit this object, it is called by SetAggregationParent
			void									SetAggregationRoot(DescriptableObject* value);

			// Return the specified aggregated base object
			DescriptableObject*						GetAggregationParent(vint index);

			// Set an aggregated base class
			void									SetAggregationParent(vint index, DescriptableObject* value);

			// Set an aggregated base class
			void									SetAggregationParent(vint index, Ptr<DescriptableObject>& value);

			// Must be called in Workflow generated classes that inherit from aggregatable C++ classes.
			void									InitializeAggregation(vint size);
#endif
			/// <summary>A function that must be called in destructors of all classes inheriting from [T:vl.reflection.AggregatableDescription`1].</summary>
			void									FinalizeAggregation();

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			template<typename T>
			void SafeAggregationCast(T*& result)
			{
				auto expected = dynamic_cast<T*>(this);
				if (expected)
				{
					CHECK_ERROR(result == nullptr, L"vl::reflection::DescriptableObject::SafeAggregationCast<T>()#Found multiple ways to do aggregation cast.");
					result = expected;
				}
				if (IsAggregated())
				{
					for (vint i = 0; i < aggregationSize; i++)
					{
						if (auto parent = GetAggregationParent(i))
						{
							parent->SafeAggregationCast<T>(result);
						}
					}
				}
			}
#endif
		public:
			DescriptableObject();
			virtual ~DescriptableObject();

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			/// <summary>
			/// <p>Get the type descriptor that describe the real type of this object.</p>
			/// </summary>
			/// <returns>The real type.</returns>
			/// <remarks>
			/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
			/// </remarks>
			description::ITypeDescriptor*			GetTypeDescriptor();
#endif

			/// <summary>
			/// Get an internal property of this object.
			/// Internal properties are totally for customization,
			/// they do not affect the object in anyway.
			/// </summary>
			/// <returns>Value of the internal property of this object.</returns>
			/// <param name="name">Name of the property.</param>
			Ptr<Object>								GetInternalProperty(const WString& name);
			/// <summary>
			/// Set an internal property of this object.
			/// Internal properties are totally for customization,
			/// they do not affect the object in anyway.
			/// </summary>
			/// <param name="name">Name of the property.</param>
			/// <param name="value">Value of the internal property of this object.</param>
			void									SetInternalProperty(const WString& name, Ptr<Object> value);
			/// <summary>Try to delete this object.</summary>
			/// <returns>Returns true if this operation succeeded. Returns false if the object refuces to be dispose.</returns>
			/// <param name="forceDisposing">Set to true to force disposing this object. If the reference counter is not 0 if you force disposing it, it will raise a [T:vl.reflection.description.ValueNotDisposableException].</param>
			bool									Dispose(bool forceDisposing);

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			/// <summary>
			/// <p>Get the aggregation root object, which is the object that inherits this object by aggregation.</p>
			/// </summary>
			/// <returns>The aggregation root object. If this object is not aggregated, or it is the root object of others, than this function return itself.</returns>
			/// <remarks>
			/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
			/// </remarks>
			DescriptableObject*						SafeGetAggregationRoot();

#endif
			/// <summary>Cast the object to another type, this is required when the object is involved in aggregation.</summary>
			/// <returns>The object with the expected type in all involved aggregated objects. It will crash when multiple objects are found to be qualified.</returns>
			/// <typeparam name="T">The expected type to cast.</typeparam>
			/// <remarks>
			/// <p>
			/// A workflow class could inherit from multiple aggregatable C++ classes.
			/// </p>
			/// <p>
			/// In order to do pointer casting correctly,
			/// this function allow you to cast from one aggregated C++ base object to another aggregated C++ base object,
			/// even when these two objects are not involved in inheriting in C++.
			/// </p>
			/// <p>
			/// When <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>on</b>, it performs dynamic_cast.
			/// </p>
			/// </remarks>
			template<typename T>
			T* SafeAggregationCast()
			{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				T* result = nullptr;
				SafeGetAggregationRoot()->SafeAggregationCast<T>(result);
				return result;
#else
				return dynamic_cast<T*>(this);
#endif
			}
		};
		
		/// <summary>
		/// <p>
		/// Inherit from this class when you want to create a reflectable class.
		/// It should be used like this:
		/// <program><code><![CDATA[
		/// class YourClass : public Description<YourClass>
		/// {
		///     ..
		/// };
		/// ]]></code></program>
		/// </p>
		/// <p>
		/// If you want YourClass to be inheritable in scripts,
		/// instead of using Description,
		/// you should use <see cref="AggregatableDescription`1"/>, like this:
		/// <program><code><![CDATA[
		/// class YourClass : public AggregatableDescription<YourClass>
		/// {
		///     ~YourClass()
		///     {
		///         FinalizeAggregation();
		///     }
		/// };
		/// ]]></code></program>
		/// </p>
		/// <p>
		/// After you complete your type,
		/// use the following macros and functions to register your class into the global type table.
		/// </p>
		/// <p>
		/// Some of the predefined type has already been registered.
		/// If your types depend on these types, you should load those types by calling some or all of them:
		/// <ul>
		///     <li>[F:vl.reflection.description.LoadPredefinedTypes]</li>
		///     <li>[F:vl.reflection.description.LoadParsingTypes]</li>
		///     <li>[F:vl.reflection.description.JsonLoadTypes]</li>
		///     <li>[F:vl.reflection.description.XmlLoadTypes]</li>
		/// </ul>
		/// But if you link <b>GacUIReflection.cpp</b> in your project and set <b>VCZH_DEBUG_NO_REFLECTION</b> to off,
		/// all types will be automatically loaded before <b>GuiMain</b> is called.
		/// </p>
		/// <p>
		/// The order does not matter, because content of types are lazily loaded.
		/// </p>
		/// <p>
		/// Everything below should be put in <b>vl::reflection::description</b> namespaces.
		/// <ol>
		///     <li>
		///         <b>(in header files)</b> Create a macro that contains all types that you want to register.
		///         Content in the list will become full names for registered type,
		///         so it is strongly recommended to use the full name.
		///         <program><code><![CDATA[
		///             #define MY_TYPELIST(F)\
		///                 F(mynamespaces::MyClass1)\
		///                 F(mynamespaces::MyClass2)\
		///         ]]></code></program>
		///     </li>
		///     <li>
		///         <b>in header files)</b> Connect type names and types:
		///         <program><code><![CDATA[
		///             MY_TYPELIST(DECL_TYPE_INFO)
		///         ]]></code></program>
		///     </li>
		///     <li>
		///         <b>(in cpp files)</b> Connect type names and types:
		///         <program><code><![CDATA[
		///             MY_TYPELIST(IMPL_CPP_TYPE_INFO)
		///         ]]></code></program>
		///     </li>
		///     <li>
		///         <b>(in cpp files)</b> Register all members:
		///         <ul>
		///             <li>
		///                 You will need to define a macro for commas, Whatever name is fine.
		///                 <program><code><![CDATA[
		///                     #define _ ,
		///                 ]]></code></program>
		///             </li>
		///             <li>
		///                 <b>enum</b>:
		///                 use <b>BEGIN_ENUM_ITEM_MERGABLE</b> instead of <b>BEGIN_ENUM_ITEM</b> if enum items are flags instead of concrete items.
		///                 <program><code><![CDATA[
		///                     BEGIN_ENUM_ITEM(Season)
		///                         ENUM_ITEM(Spring)
		///                         ENUM_ITEM(Summer)
		///                         ENUM_ITEM(Autumn)
		///                         ENUM_ITEM(Winter)
		///                     END_ENUM_ITEM(Season)
		///                 ]]></code></program>
		///             </li>
		///             <li>
		///                 <b>enum class:</b>
		///                 use <b>BEGIN_ENUM_ITEM_MERGABLE</b> instead of <b>BEGIN_ENUM_ITEM</b> if enum items are flags instead of concrete items.
		///                 <program><code><![CDATA[
		///                     BEGIN_ENUM_ITEM(Season)
		///                         ENUM_CLASS_ITEM(Spring)
		///                         ENUM_CLASS_ITEM(Summer)
		///                         ENUM_CLASS_ITEM(Autumn)
		///                         ENUM_CLASS_ITEM(Winter)
		///                     END_ENUM_ITEM(Season)
		///                 ]]></code></program>
		///             </li>
		///             <li>
		///                 <b>struct</b>:
		///                 It doesn't necessary mean a struct in C++.
		///                 Structs in reflection and Workflow script mean value types that carry only data, without methods and inheritance.
		///                 <program><code><![CDATA[
		///                     BEGIN_STRUCT_MEMBER(Point)
		///                         STRUCT_MEMBER(x)
		///                         STRUCT_MEMBER(y)
		///                     END_STRUCT_MEMBER(Point)
		///                 ]]></code></program>
		///             </li>
		///             <li>
		///                 <p>
		///                 <b>class</b>:
		///                 It doesn't necessary mean a class in C++.
		///                 Classes in reflection and Workflow script mean reference types.
		///                 </p>
		///                 <p>
		///                 Here are all macros that register content of classes
		///                 <ul>
		///                     <li>CLASS_MEMBER_BASE</li>
		///                     <li>CLASS_MEMBER_FIELD</li>
		///                     <li>CLASS_MEMBER_CONSTRUCTOR</li>
		///                     <li>CLASS_MEMBER_EXTERNALCTOR(_TEMPLATE)?</li>
		///                     <li>CLASS_MEMBER_METHOD(_OVERLOAD)?_RENAME</li>
		///                     <li>CLASS_MEMBER_(STATIC_)?METHOD(_OVERLOAD)?</li>
		///                     <li>CLASS_MEMBER_(STATIC_)?EXTERNALMETHOD(_TEMPLATE)?</li>
		///                     <li>CLASS_MEMBER_PROPERTY(_EVENT)?(_READONLY)?(_FAST)?</li>
		///                     <li>CLASS_MEMBER_PROPERTY_REFERENCETEMPLATE</li>
		///                     <li>CLASS_MEMBER_EVENT</li>
		///                 </ul>
		///                 </p>
		///                 <p>
		///                 <program><code><![CDATA[
		///                     BEGIN_CLASS_MEMBER(MyClass)
		///
		///                         // 01) Declare a base class (can have multiple base classes).
		///                         CLASS_MEMBER_BASE(MyBaseClass)
		///
		///                         // 02) Declare a field.
		///                         CLASS_MEMBER_FIELD(myField)
		///
		///                         // 03) Default constructor that results in a raw pointer.
		///                         CLASS_MEMBER_CONSTRUCTIOR(MyClass*(), NO_PARAMETER)
		///
		///                         // 04) Default constructor that results in a shared pointer.
		///                         CLASS_MEMBER_CONSTRUCTIOR(Ptr<MyClass>(), NO_PARAMETER)
		///
		///                         // 05) Constructor with arguments.
		///                         CLASS_MEMBER_CONSTRUCTOR(Ptr<MyClass>(int, const WString&), {L"numberParameter" _ L"stringParameter"})
		///
		///                         // 06) Inject a global function as a constructor.
		///                         CLASS_MEMBER_EXTERNALCTOR(Ptr<MyClass>(int, const WString&), {L"numberParameter" _ L"stringParameter"}, mynamespace::CreateMyClass)
		///
		///                         // 07) Inject a consturctor and specify how to generate C++ code, "*" means not able to generate.
		///                         CLASS_MEMBER_EXTERNALCTOR_TEMPLATE(Ptr<MyClass>(int, const WString&), {L"numberParameter" _ L"stringParameter"}, CreateMyClass, L"mynamespace::GetMyClass($Arguments)", L"::vl::Func<$Func>(&mynamespace::GetMyClass)")
		///                         CLASS_MEMBER_EXTERNALCTOR_TEMPLATE(Ptr<MyClass>(), NO_PARAMETER, []()->Ptr<MyClass>{return nullptr;}, L"*", L"*")
		///
		///                         // 08) Add unoverloaded functions.
		///                         CLASS_MEMBER_METHOD(MyFunction1, NO_PARAMETER)
		///                         CLASS_MEMBER_METHOD(MyFunction2, {L"parameter1" _ L"parameter2"})
		///
		///                         // 09) Add unoverloaded functions but give different names. Unoverloaded only means in C++, not in renamed functions.
		///                         CLASS_MEMBER_METHOD_RENAME(MyNewName1, MyFunction1, NO_PARAMETER)
		///                         CLASS_MEMBER_METHOD_RENAME(MyNewName2, MyFunction2, {L"parameter1" _ L"parameter2"})
		///
		///                         // 10) Add overloaded functions, with function type specified in method pointers
		///                         CLASS_MEMBER_METHOD_OVERLOAD(MyFunction3, NO_PARAMETER, int(MyClass::*)())
		///                         CLASS_MEMBER_METHOD_OVERLOAD(MyFunction3, {L"parameter"}, int(MyClass::*)(int))
		///                         CLASS_MEMBER_METHOD_OVERLOAD(MyFunction3, {L"parameter1" _ L"parameter2"}, int(MyClass::*)(int, const WString&))
		///
		///                         // 11) Add overloaded functions but give different names.
		///                         CLASS_MEMBER_METHOD_OVERLOAD_RENAME(MyNewName3, MyFunction3, NO_PARAMETER, int(MyClass::*)())
		///                         CLASS_MEMBER_METHOD_OVERLOAD_RENAME(MyNewName4, MyFunction3, {L"parameter"}, int(MyClass::*)(int))
		///                         CLASS_MEMBER_METHOD_OVERLOAD_RENAME(MyNewName4, MyFunction3, {L"parameter1" _ L"parameter2"}, int(MyClass::*)(int, const WString&))
		///
		///                         // 12) Inject global functions as methods:
		///                         CLASS_MEMBER_EXTERNALMETHOD(MyNewName5, {L"parameter"}, int(MyClass::*)(int), mynamespace::AGlobalFunction)
		///
		///                         // 13) Inject a method and specify how to generate C++ code, "*" means not able to generate.
		///                         CLASS_MEMBER_EXTERNALMETHOD_TEMPLATE(MyNewName5, {L"parameter1" _ L"parameter2"}, int(MyClass::*)(int, const WString&), [](MyClass* a, int b, const WString& c){return 0;}, L"*", L"*")
		///
		///                         // 14) Add unoverloaded static functions
		///                         CLASS_MEMBER_STATIC_METHOD(MyFunction4, NO_PARAMETER)
		///                         CLASS_MEMBER_STATIC_METHOD(MyFunction5, {L"parameter1" _ L"parameter2"})
		///
		///                         // 15) Add overloaded static functions
		///                         CLASS_MEMBER_STATIC_METHOD_OVERLOAD(MyFunction6, NO_PARAMETER, int(*)())
		///                         CLASS_MEMBER_STATIC_METHOD_OVERLOAD(MyFunction6, {L"parameter"}, int(*)(int))
		///                         CLASS_MEMBER_STATIC_METHOD_OVERLOAD(MyFunction6, {L"parameter1" _ L"parameter2"}, int(*)(int, const WString&))
		///
		///                         // 16) Inject global functions as static methods:
		///                         CLASS_MEMBER_STATIC_EXTERNALMETHOD(MyNewName6, {L"parameter"}, int(*)(int), mynamespace::AGlobalFunction2)
		///
		///                         // 17) Inject a static method and specify how to generate C++ code, "*" means not able to generate.
		///                         CLASS_MEMBER_STATIC_EXTERNALMETHOD_TEMPLATE(MyNewName6, {L"parameter1" _ L"parameter2"}, int(*)(int, const WString&), [](int b, const WString& c){return 0;}, L"*")
		///
		///                         // 18) Add a getter function as a property
		///                         CLASS_MEMBER_PROPERTY_READONLY_FAST(X)
		///                         // which is short for
		///                         CLASS_MEMBER_METHOD(GetX, NO_PARAMETER)
		///                         CLASS_MEMBER_PROPERTY_READONLY(X, GetX)
		///
		///                         // 19) Add a pair of getter and setter functions as a property
		///                         CLASS_MEMBER_PROPERTY_FAST(X)
		///                         // which is short for
		///                         CLASS_MEMBER_METHOD(GetX, NO_PARAMETER)
		///                         CLASS_MEMBER_METHOD(SetX, {L"value"})
		///                         CLASS_MEMBER_PROPERTY(X, GetX, SetX)
		///
		///                         // 20) Add a getter function as a property with a property changed event
		///                         CLASS_MEMBER_EVENT(XChanged)
		///                         CLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST(X, XChanged)
		///                         // which is short for
		///                         CLASS_MEMBER_EVENT(XChanged)
		///                         CLASS_MEMBER_METHOD(GetX, NO_PARAMETER)
		///                         CLASS_MEMBER_PROPERTY_EVENT_READONLY(X, GetX, XChanged)
		///
		///                         // 21) Add a pair of getter and setter functions as a property with a property changed event
		///                         CLASS_MEMBER_EVENT(XChanged)
		///                         CLASS_MEMBER_PROPERTY_EVENT_FAST(X, XChanged)
		///                         // which is short for
		///                         CLASS_MEMBER_EVENT(XChanged)
		///                         CLASS_MEMBER_METHOD(GetX, NO_PARAMETER)
		///                         CLASS_MEMBER_METHOD(SetX, {L"value"})
		///                         CLASS_MEMBER_PROPERTY_EVENT(X, GetX, SetX, XChanged)
		///
		///                     END_CLASS_MEMBER(MyClass)
		///                 ]]></code></program>
		///                 </p>
		///                 <p>
		///                 If the code compiles, the class should look like this:
		///                 <program><code><![CDATA[
		///                     class MyClass : public Description<MyClass>
		///                     {
		///                     public:
		///                         MyClass();
		///                         MyClass(int numberParameter, const WString& stringParameter);
		///
		///                         int MyFunction1();
		///                         int MyFunction2(int parameter1, const WString& parameter2);
		///                         int MyFunction3();
		///                         int MyFunction3(int parameter);
		///                         int MyFunction3(int parameter1, const WString& parameter2);
		///
		///                         static int MyFunction4();
		///                         static int MyFunction5(int parameter1, const WString& parameter2);
		///                         static int MyFunction6();
		///                         static int MyFunction6(int parameter);
		///                         static int MyFunction6(int parameter1, const WString& parameter2);
		///
		///                         Event<void()> XChanged;
		///                         int GetX();
		///                         void SetX(int value);
		///                     };
		///
		///                     Ptr<MyClass> CreateMyClass(int numberParameter, const WString7 stringParameter);
		///                     int GlobalFunction(MyClass* self, int parameter);
		///                 ]]></code></program>
		///                 </p>
		///             </li>
		///             <li>
		///                 <p>
		///                 <b>interface</b>:
		///                 A C++ class can be registered as a reflectable interface if:
		///                 <ul>
		///                     <li>Directly or indirectly inherits [T:vl.reflection.IDescriptable]</li>
		///                     <li>The only registered constructor (if exists) should use Ptr&lt;[T:vl.reflection.description.IValueInterfaceProxy]&gt; as a parameter, so that a Workflow script class could implement this interface.</li>
		///                 </ul>
		///                 </p>
		///                 <p>
		///                 Suppose you have an interface like this:
		///                 <program><code><![CDATA[
		///                     class IMyInterface : public virtual IDescriptable, public Description<IMyInterface>
		///                     {
		///                     public:
		///                         int GetX();
		///                         void SetX(int value);
		///                     };
		///                 ]]></code></program>
		///                 </p>
		///                 <p>
		///                 If you want to allow a Workflow script class implement this interface, you should first add a proxy like this:
		///                 <program><code><![CDATA[
		///                     #pragma warning(push)
		///                     #pragma warning(disable:4250)
		///                     BEGIN_INTERFACE_PROXY_NOPARENT_RAWPTR(IMyInterface)
		///                         // or BEGIN_INTERFACE_PROXY_RAWPTR(IMyInterface, baseInterfaces...)
		///                         // or BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(IMyInterface)
		///                         // or BEGIN_INTERFACE_PROXY_SHAREDPTR(IMyInterface, baseInterfaces...)
		///                         int GetX()override
		///                         {
		///                             INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetX)
		///                         }
		///
		///                         void SetX(int value)override
		///                         {
		///                             INVOKE_INTERFACE_PROXY(SetX, value)
		///                         }
		///                     END_INTERFACE_PROXY(IMyInterface)
		///                     #pragma warning(pop)
		///                 ]]></code></program>
		///                 </p>
		///                 <p>
		///                 And then use this code to register the interface:
		///                 <program><code><![CDATA[
		///                     BEGIN_INTERFACE_MEMBER(IMyInterface)
		///                         ...
		///                     END_INTERFACE_MEMBER(IMyInterface)
		///                 ]]></code></program>
		///                 </p>
		///                 <p>
		///                 Everything else is the same as registering classes.
		///                 Use <b>BEGIN_INTERFACE_MEMBER_NOPROXY</b> to register an interface without a proxy,
		///                 which means a Workflow script class cannot implement this interface.
		///                 </p>
		///             </li>
		///             <li>
		///                 Undefine the macro for comma:
		///                 <program><code><![CDATA[
		///                     #undef _
		///                 ]]></code></program>
		///             </li>
		///         </ul>
		///     </li>
		///     <li>
		///         <b>(in cpp files)</b> Create a type loader:
		///         <program><code><![CDATA[
		///             class MyTypeLoader : public Object, public ITypeLoader
		///             {
		///             public:
		///                 void Load(ITypeManager* manager)
		///                 {
		///                     MY_TYPELIST(ADD_TYPE_INFO)
		///                 }
		///
		///                 void Unload(ITypeManager* manager)
		///                 {
		///                 }
		///             };
		///         ]]></code></program>
		///     </li>
		///     <li>
		///         Before using reflection on registered types, you need to register the type loader:
		///         <program><code><![CDATA[
		///             vl::reflection::description::GetGlobalTypeManager()->AddTypeLoader(new MyTypeLoader);
		///         ]]></code></program>
		///     </li>
		/// </ol>
		/// </p>
		/// </summary>
		/// <typeparam name="T">Type that inherit this class.</typeparam>
		template<typename T>
		class Description : public virtual DescriptableObject
		{
		protected:
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			static description::ITypeDescriptor*		associatedTypeDescriptor;
#endif
		public:
			Description()
			{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

				if(objectSize<sizeof(T))
				{
					objectSize=sizeof(T);
					if(!typeDescriptor || !*typeDescriptor || associatedTypeDescriptor)
					{
						typeDescriptor=&associatedTypeDescriptor;
					}
				}
#endif
			}

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			static description::ITypeDescriptor* GetAssociatedTypeDescriptor()
			{
				return associatedTypeDescriptor;
			}

			static void SetAssociatedTypeDescroptor(description::ITypeDescriptor* typeDescroptor)
			{
				associatedTypeDescriptor=typeDescroptor;
			}
#endif
		};

		/// <summary>
		/// Inherit from this class when you want to create a reflectable class that can be inherited by Workflow script classes.
		/// </summary>
		/// <typeparam name="T">Type that inherit this class.</typeparam>
		template<typename T>
		class AggregatableDescription : public Description<T>
		{
		};

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
		template<typename T>
		description::ITypeDescriptor* Description<T>::associatedTypeDescriptor=0;
#endif

		/// <summary>Base type of all reflectable interfaces. All reflectable interface types should be virtual inherited.</summary>
		class IDescriptable : public virtual Interface, public Description<IDescriptable>
		{
		public:
			~IDescriptable(){}
		};

/***********************************************************************
ReferenceCounterOperator
***********************************************************************/
	}

	template<typename T>
	struct ReferenceCounterOperator<T, typename PointerConvertable<T, reflection::DescriptableObject>::YesNoType>
	{
		static __forceinline volatile vint* CreateCounter(T* reference)
		{
			reflection::DescriptableObject* obj=reference;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			if (obj->IsAggregated())
			{
				if (auto root = obj->GetAggregationRoot())
				{
					return &root->referenceCounter;
				}
			}
#endif
			return &obj->referenceCounter;
		}

		static __forceinline void DeleteReference(volatile vint* counter, void* reference)
		{
			reflection::DescriptableObject* obj=(T*)reference;
			obj->Dispose(false);
		}
	};

	namespace reflection
	{

		namespace description
		{

/***********************************************************************
Value
***********************************************************************/

			class IBoxedValue : public virtual IDescriptable, public Description<IBoxedValue>
			{
			public:
				enum CompareResult
				{
					Smaller,
					Greater,
					Equal,
					NotComparable,
				};

				virtual Ptr<IBoxedValue>		Copy() = 0;
				virtual CompareResult			ComparePrimitive(Ptr<IBoxedValue> boxedValue) = 0;
			};

			/// <summary>A type to store all values of reflectable types.</summary>
			/// <remarks>
			/// To convert between <b>Value</b> and its real C++ type, the following functions are recommended:
			/// <ul>
			///     <li>[F:vl.reflection.description.BoxValue`1]</li>
			///     <li>[F:vl.reflection.description.UnboxValue`1]</li>
			///     <li>[F:vl.reflection.description.BoxParameter`1]</li>
			///     <li>[F:vl.reflection.description.UnboxParameter`1]</li>
			/// </ul>
			/// </remarks>
			class Value : public Object
			{
			public:
				/// <summary>How the value is stored.</summary>
				enum ValueType
				{
					/// <summary>The value is null.</summary>
					Null,
					/// <summary>The reference value is stored using a raw pointer.</summary>
					RawPtr,
					/// <summary>The reference value is stored using a shared pointer.</summary>
					SharedPtr,
					/// <summary>The value is stored by boxing.</summary>
					BoxedValue,
				};
			protected:
				ValueType						valueType;
				DescriptableObject*				rawPtr;
				Ptr<DescriptableObject>			sharedPtr;
				Ptr<IBoxedValue>				boxedValue;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				ITypeDescriptor*				typeDescriptor;
#endif

				Value(DescriptableObject* value);
				Value(Ptr<DescriptableObject> value);
				Value(Ptr<IBoxedValue> value, ITypeDescriptor* associatedTypeDescriptor);

				vint							Compare(const Value& a, const Value& b)const;
			public:
				/// <summary>Create a null value.</summary>
				Value();
				Value(const Value& value);
				Value&							operator=(const Value& value);
				bool							operator==(const Value& value)const { return Compare(*this, value) == 0; }
				bool							operator!=(const Value& value)const { return Compare(*this, value) != 0; }
				bool							operator<(const Value& value)const { return Compare(*this, value)<0; }
				bool							operator<=(const Value& value)const { return Compare(*this, value) <= 0; }
				bool							operator>(const Value& value)const { return Compare(*this, value)>0; }
				bool							operator>=(const Value& value)const { return Compare(*this, value) >= 0; }

				/// <summary>Find out how the value is stored.</summary>
				/// <returns>Returns How the value is stored.</returns>
				ValueType						GetValueType()const;
				/// <summary>Get the stored raw pointer if <b>GetValueType()</b> returns <b>RawPtr</b> or <b>SharedPtr</b>.</summary>
				/// <returns>The stored raw pointer. Returns null if failed.</returns>
				DescriptableObject*				GetRawPtr()const;
				/// <summary>Get the stored shared pointer if <b>GetValueType()</b> returns <b>SharedPtr</b>.</summary>
				/// <returns>The stored shared pointer. Returns null if failed.</returns>
				Ptr<DescriptableObject>			GetSharedPtr()const;
				/// <summary>Get the stored value if <b>GetValueType()</b> returns <b>BoxedValue</b>.</summary>
				/// <returns>The stored text. Returns empty if failed.</returns>
				Ptr<IBoxedValue>				GetBoxedValue()const;
				/// <summary>Test if this value isnull.</summary>
				/// <returns>Returns true if this value is null.</returns>
				bool							IsNull()const;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				/// <summary>Get the real type of the stored object.</summary>
				/// <returns>The real type. Returns null if the value is null.</returns>
				/// <remarks>
				/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
				/// </remarks>
				ITypeDescriptor*				GetTypeDescriptor()const;
				WString							GetTypeFriendlyName()const;
				bool							CanConvertTo(ITypeDescriptor* targetType, ValueType targetValueType)const;
				bool							CanConvertTo(ITypeInfo* targetType)const;
#endif

				/// <summary>Create a value from a raw pointer.</summary>
				/// <returns>The created value.</returns>
				/// <param name="value">The raw pointer to store.</param>
				static Value					From(DescriptableObject* value);
				/// <summary>Create a value from a shared pointer.</summary>
				/// <returns>The created value.</returns>
				/// <param name="value">The shared pointer to store.</param>
				static Value					From(Ptr<DescriptableObject> value);
				/// <summary>Create a boxed value.</summary>
				/// <returns>The created value.</returns>
				/// <param name="value">The boxed value to store.</param>
				/// <param name="type">The type of the boxed value.</param>
				static Value					From(Ptr<IBoxedValue> value, ITypeDescriptor* type);

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				static IMethodInfo*				SelectMethod(IMethodGroupInfo* methodGroup, collections::Array<Value>& arguments);

				/// <summary>Call the default constructor of the specified type to create a value.</summary>
				/// <returns>The created value.</returns>
				/// <param name="type">The type to create the value.</param>
				/// <remarks>
				/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
				/// </remarks>
				static Value					Create(ITypeDescriptor* type);

				/// <summary>Call the constructor of the specified type to create a value.</summary>
				/// <returns>The created value.</returns>
				/// <param name="type">The type to create the value.</param>
				/// <param name="arguments">Arguments for the constructor.</param>
				/// <remarks>
				/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
				/// </remarks>
				/// <example><![CDATA[
				/// // reflectable C++ types
				/// 
				/// namespace mynamespace
				/// {
				///     class MyClass : public Object, public Description<MyClass>
				///     {
				///     public:
				///         MyClass()
				///             :data(L"Hello, world!")
				///         {
				///         }
				/// 
				///         MyClass(const WString& _data)
				///             :data(_data)
				///         {
				///         }
				/// 
				///         WString data;
				///     };
				/// }
				/// 
				/// #define MY_TYPELIST(F)\
				///     F(mynamespace::MyClass)\
				/// 
				/// // it is recommended to put the content below in a separated header file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             MY_TYPELIST(DECL_TYPE_INFO)
				///         }
				///     }
				/// }
				/// 
				/// // it is recommended to put the content below in a separated cpp file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             using namespace mynamespace;
				/// 
				/// #define _ ,
				/// 
				///             MY_TYPELIST(IMPL_CPP_TYPE_INFO)
				/// 
				///             BEGIN_CLASS_MEMBER(MyClass)
				///                 CLASS_MEMBER_CONSTRUCTOR(Ptr<MyClass>(), NO_PARAMETER)
				///                 CLASS_MEMBER_CONSTRUCTOR(Ptr<MyClass>(const WString&), { L"data" })
				///             END_CLASS_MEMBER(MyClass)
				/// 
				/// #undef  _
				///         }
				///     }
				/// }
				/// 
				/// class MyTypeLoader : public Object, public ITypeLoader
				/// {
				/// public:
				///     void Load(ITypeManager* manager)
				///     {
				///         MY_TYPELIST(ADD_TYPE_INFO)
				///     }
				/// 
				///     void Unload(ITypeManager* manager)
				///     {
				///     }
				/// };
				/// 
				/// // main function
				/// 
				/// int main()
				/// {
				///     LoadPredefinedTypes();
				///     GetGlobalTypeManager()->AddTypeLoader(new MyTypeLoader);
				///     GetGlobalTypeManager()->Load();
				///     {
				///         auto myClass = Value::Create(GetTypeDescriptor(L"mynamespace::MyClass"), (Value_xs(), WString(L"Hello, world!!!")));
				/// 
				///         auto ptrMyClass1 = UnboxValue<Ptr<MyClass>>(myClass);
				///         Console::WriteLine(ptrMyClass1->data);
				/// 
				///         Ptr<MyClass> ptrMyClass2;
				///         UnboxParameter(myClass, ptrMyClass2);
				///         Console::WriteLine(ptrMyClass2->data);
				///     }
				///     DestroyGlobalTypeManager();
				/// }
				/// ]]></example>
				static Value					Create(ITypeDescriptor* type, collections::Array<Value>& arguments);

				/// <summary>Call the default constructor of the specified type to create a value.</summary>
				/// <returns>The created value.</returns>
				/// <param name="typeName">The registered full name for the type to create the value.</param>
				/// <remarks>
				/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
				/// </remarks>
				static Value					Create(const WString& typeName);

				/// <summary>Call the constructor of the specified type to create a value.</summary>
				/// <returns>The created value.</returns>
				/// <param name="typeName">The registered full name for the type to create the value.</param>
				/// <param name="arguments">Arguments for the constructor.</param>
				/// <remarks>
				/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
				/// </remarks>
				/// <example><![CDATA[
				/// // reflectable C++ types
				/// 
				/// namespace mynamespace
				/// {
				///     class MyClass : public Object, public Description<MyClass>
				///     {
				///     public:
				///         MyClass()
				///             :data(L"Hello, world!")
				///         {
				///         }
				/// 
				///         MyClass(const WString& _data)
				///             :data(_data)
				///         {
				///         }
				/// 
				///         WString data;
				///     };
				/// }
				/// 
				/// #define MY_TYPELIST(F)\
				///     F(mynamespace::MyClass)\
				/// 
				/// // it is recommended to put the content below in a separated header file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             MY_TYPELIST(DECL_TYPE_INFO)
				///         }
				///     }
				/// }
				/// 
				/// // it is recommended to put the content below in a separated cpp file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             using namespace mynamespace;
				/// 
				/// #define _ ,
				/// 
				///             MY_TYPELIST(IMPL_CPP_TYPE_INFO)
				/// 
				///             BEGIN_CLASS_MEMBER(MyClass)
				///                 CLASS_MEMBER_CONSTRUCTOR(Ptr<MyClass>(), NO_PARAMETER)
				///                 CLASS_MEMBER_CONSTRUCTOR(Ptr<MyClass>(const WString&), { L"data" })
				///             END_CLASS_MEMBER(MyClass)
				/// 
				/// #undef  _
				///         }
				///     }
				/// }
				/// 
				/// class MyTypeLoader : public Object, public ITypeLoader
				/// {
				/// public:
				///     void Load(ITypeManager* manager)
				///     {
				///         MY_TYPELIST(ADD_TYPE_INFO)
				///     }
				/// 
				///     void Unload(ITypeManager* manager)
				///     {
				///     }
				/// };
				/// 
				/// // main function
				/// 
				/// int main()
				/// {
				///     LoadPredefinedTypes();
				///     GetGlobalTypeManager()->AddTypeLoader(new MyTypeLoader);
				///     GetGlobalTypeManager()->Load();
				///     {
				///         auto myClass = Value::Create(L"mynamespace::MyClass", (Value_xs(), WString(L"Hello, world!!!")));
				/// 
				///         auto ptrMyClass1 = UnboxValue<Ptr<MyClass>>(myClass);
				///         Console::WriteLine(ptrMyClass1->data);
				/// 
				///         Ptr<MyClass> ptrMyClass2;
				///         UnboxParameter(myClass, ptrMyClass2);
				///         Console::WriteLine(ptrMyClass2->data);
				///     }
				///     DestroyGlobalTypeManager();
				/// }
				/// ]]></example>
				static Value					Create(const WString& typeName, collections::Array<Value>& arguments);

				/// <summary>Call a static method of the specified type.</summary>
				/// <returns>The return value from that method.</returns>
				/// <param name="typeName">The registered full name for the type.</param>
				/// <param name="name">The registered name for the method.</param>
				/// <remarks>
				/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
				/// </remarks>
				static Value					InvokeStatic(const WString& typeName, const WString& name);

				/// <summary>Call a static method of the specified type.</summary>
				/// <returns>The return value from that method.</returns>
				/// <param name="typeName">The registered full name for the type.</param>
				/// <param name="name">The registered name for the method.</param>
				/// <param name="arguments">Arguments for the method.</param>
				/// <remarks>
				/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
				/// </remarks>
				/// <example><![CDATA[
				/// // reflectable C++ types
				/// 
				/// namespace mynamespace
				/// {
				///     class MyClass : public Object, public Description<MyClass>
				///     {
				///     public:
				///         static void PrintHelloWorld(const WString& name)
				///         {
				///             Console::WriteLine(L"Hello, " + name + L"!");
				///         }
				///     };
				/// }
				/// 
				/// #define MY_TYPELIST(F)\
				///     F(mynamespace::MyClass)\
				/// 
				/// // it is recommended to put the content below in a separated header file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             MY_TYPELIST(DECL_TYPE_INFO)
				///         }
				///     }
				/// }
				/// 
				/// // it is recommended to put the content below in a separated cpp file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             using namespace mynamespace;
				/// 
				/// #define _ ,
				/// 
				///             MY_TYPELIST(IMPL_CPP_TYPE_INFO)
				/// 
				///             BEGIN_CLASS_MEMBER(MyClass)
				///                 CLASS_MEMBER_STATIC_METHOD(PrintHelloWorld, { L"name" })
				///             END_CLASS_MEMBER(MyClass)
				/// 
				/// #undef  _
				///         }
				///     }
				/// }
				/// 
				/// class MyTypeLoader : public Object, public ITypeLoader
				/// {
				/// public:
				///     void Load(ITypeManager* manager)
				///     {
				///         MY_TYPELIST(ADD_TYPE_INFO)
				///     }
				/// 
				///     void Unload(ITypeManager* manager)
				///     {
				///     }
				/// };
				/// 
				/// // main function
				/// 
				/// int main()
				/// {
				///     LoadPredefinedTypes();
				///     GetGlobalTypeManager()->AddTypeLoader(new MyTypeLoader);
				///     GetGlobalTypeManager()->Load();
				///     {
				///         Value::InvokeStatic(L"mynamespace::MyClass", L"PrintHelloWorld", (Value_xs(), WString(L"Gaclib")));
				///     }
				///     DestroyGlobalTypeManager();
				/// }
				/// ]]></example>
				static Value					InvokeStatic(const WString& typeName, const WString& name, collections::Array<Value>& arguments);

				/// <summary>Call the getter function for a property.</summary>
				/// <returns>The value of the property.</returns>
				/// <param name="name">The registered name for the property.</param>
				/// <remarks>
				/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
				/// </remarks>
				/// <example><![CDATA[
				/// // reflectable C++ types
				/// 
				/// namespace mynamespace
				/// {
				///     class MyClass : public Object, public Description<MyClass>
				///     {
				///     private:
				///         WString prop;
				///     public:
				///         WString field;
				/// 
				///         WString GetProp() { return prop; };
				///         void SetProp(const WString& value) { prop = value; }
				///     };
				/// }
				/// 
				/// #define MY_TYPELIST(F)\
				///     F(mynamespace::MyClass)\
				/// 
				/// // it is recommended to put the content below in a separated header file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             MY_TYPELIST(DECL_TYPE_INFO)
				///         }
				///     }
				/// }
				/// 
				/// // it is recommended to put the content below in a separated cpp file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             using namespace mynamespace;
				/// 
				/// #define _ ,
				/// 
				///             MY_TYPELIST(IMPL_CPP_TYPE_INFO)
				/// 
				///             BEGIN_CLASS_MEMBER(MyClass)
				///                 CLASS_MEMBER_CONSTRUCTOR(Ptr<MyClass>(), NO_PARAMETER)
				///                 CLASS_MEMBER_FIELD(field)
				///                 CLASS_MEMBER_PROPERTY_FAST(Prop)
				///             END_CLASS_MEMBER(MyClass)
				/// 
				/// #undef  _
				///         }
				///     }
				/// }
				/// 
				/// class MyTypeLoader : public Object, public ITypeLoader
				/// {
				/// public:
				///     void Load(ITypeManager* manager)
				///     {
				///         MY_TYPELIST(ADD_TYPE_INFO)
				///     }
				/// 
				///     void Unload(ITypeManager* manager)
				///     {
				///     }
				/// };
				/// 
				/// // main function
				/// 
				/// int main()
				/// {
				///     LoadPredefinedTypes();
				///     GetGlobalTypeManager()->AddTypeLoader(new MyTypeLoader);
				///     GetGlobalTypeManager()->Load();
				///     {
				///         auto td = GetTypeDescriptor(L"mynamespace::MyClass");
				///         auto myClass = Value::Create(td);
				/// 
				///         myClass.SetProperty(L"field", BoxValue<WString>(L"Hello, world!"));
				///         myClass.SetProperty(L"Prop", BoxValue<WString>(L"Hello, Gaclib!"));
				/// 
				///         Console::WriteLine(UnboxValue<WString>(myClass.GetProperty(L"field")));
				///         Console::WriteLine(UnboxValue<WString>(myClass.GetProperty(L"Prop")));
				///     }
				///     DestroyGlobalTypeManager();
				/// }
				/// ]]></example>
				Value							GetProperty(const WString& name)const;

				/// <summary>Call the setter function for a property.</summary>
				/// <param name="name">The registered name for the property.</param>
				/// <param name="newValue">The value to set the property.</param>
				/// <remarks>
				/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
				/// </remarks>
				/// <example><![CDATA[
				/// // reflectable C++ types
				/// 
				/// namespace mynamespace
				/// {
				///     class MyClass : public Object, public Description<MyClass>
				///     {
				///     private:
				///         WString prop;
				///     public:
				///         WString field;
				/// 
				///         WString GetProp() { return prop; };
				///         void SetProp(const WString& value) { prop = value; }
				///     };
				/// }
				/// 
				/// #define MY_TYPELIST(F)\
				///     F(mynamespace::MyClass)\
				/// 
				/// // it is recommended to put the content below in a separated header file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             MY_TYPELIST(DECL_TYPE_INFO)
				///         }
				///     }
				/// }
				/// 
				/// // it is recommended to put the content below in a separated cpp file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             using namespace mynamespace;
				/// 
				/// #define _ ,
				/// 
				///             MY_TYPELIST(IMPL_CPP_TYPE_INFO)
				/// 
				///             BEGIN_CLASS_MEMBER(MyClass)
				///                 CLASS_MEMBER_CONSTRUCTOR(Ptr<MyClass>(), NO_PARAMETER)
				///                 CLASS_MEMBER_FIELD(field)
				///                 CLASS_MEMBER_PROPERTY_FAST(Prop)
				///             END_CLASS_MEMBER(MyClass)
				/// 
				/// #undef  _
				///         }
				///     }
				/// }
				/// 
				/// class MyTypeLoader : public Object, public ITypeLoader
				/// {
				/// public:
				///     void Load(ITypeManager* manager)
				///     {
				///         MY_TYPELIST(ADD_TYPE_INFO)
				///     }
				/// 
				///     void Unload(ITypeManager* manager)
				///     {
				///     }
				/// };
				/// 
				/// // main function
				/// 
				/// int main()
				/// {
				///     LoadPredefinedTypes();
				///     GetGlobalTypeManager()->AddTypeLoader(new MyTypeLoader);
				///     GetGlobalTypeManager()->Load();
				///     {
				///         auto td = GetTypeDescriptor(L"mynamespace::MyClass");
				///         auto myClass = Value::Create(td);
				/// 
				///         myClass.SetProperty(L"field", BoxValue<WString>(L"Hello, world!"));
				///         myClass.SetProperty(L"Prop", BoxValue<WString>(L"Hello, Gaclib!"));
				/// 
				///         Console::WriteLine(UnboxValue<WString>(myClass.GetProperty(L"field")));
				///         Console::WriteLine(UnboxValue<WString>(myClass.GetProperty(L"Prop")));
				///     }
				///     DestroyGlobalTypeManager();
				/// }
				/// ]]></example>
				void							SetProperty(const WString& name, const Value& newValue);

				/// <summary>Call a non-static method.</summary>
				/// <returns>The return value from that method.</returns>
				/// <param name="name">The registered name for the method.</param>
				/// <remarks>
				/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
				/// </remarks>
				Value							Invoke(const WString& name)const;

				/// <summary>Call a non-static method.</summary>
				/// <returns>The return value from that method.</returns>
				/// <param name="name">The registered name for the method.</param>
				/// <param name="arguments">Arguments for the method.</param>
				/// <remarks>
				/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
				/// </remarks>
				/// <example><![CDATA[
				/// // reflectable C++ types
				/// 
				/// namespace mynamespace
				/// {
				///     class MyClass : public Object, public Description<MyClass>
				///     {
				///     public:
				///         void PrintHelloWorld(const WString& name)
				///         {
				///             Console::WriteLine(L"Hello, " + name + L"!");
				///         }
				///     };
				/// }
				/// 
				/// #define MY_TYPELIST(F)\
				///     F(mynamespace::MyClass)\
				/// 
				/// // it is recommended to put the content below in a separated header file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             MY_TYPELIST(DECL_TYPE_INFO)
				///         }
				///     }
				/// }
				/// 
				/// // it is recommended to put the content below in a separated cpp file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             using namespace mynamespace;
				/// 
				/// #define _ ,
				/// 
				///             MY_TYPELIST(IMPL_CPP_TYPE_INFO)
				/// 
				///             BEGIN_CLASS_MEMBER(MyClass)
				///                 CLASS_MEMBER_CONSTRUCTOR(Ptr<MyClass>(), NO_PARAMETER)
				///                 CLASS_MEMBER_METHOD(PrintHelloWorld, { L"name" })
				///             END_CLASS_MEMBER(MyClass)
				/// 
				/// #undef  _
				///         }
				///     }
				/// }
				/// 
				/// class MyTypeLoader : public Object, public ITypeLoader
				/// {
				/// public:
				///     void Load(ITypeManager* manager)
				///     {
				///         MY_TYPELIST(ADD_TYPE_INFO)
				///     }
				/// 
				///     void Unload(ITypeManager* manager)
				///     {
				///     }
				/// };
				/// 
				/// // main function
				/// 
				/// int main()
				/// {
				///     LoadPredefinedTypes();
				///     GetGlobalTypeManager()->AddTypeLoader(new MyTypeLoader);
				///     GetGlobalTypeManager()->Load();
				///     {
				///         auto td = GetTypeDescriptor(L"mynamespace::MyClass");
				///         auto myClass = Value::Create(td);
				///         myClass.Invoke(L"PrintHelloWorld", (Value_xs(), WString(L"Gaclib")));
				///     }
				///     DestroyGlobalTypeManager();
				/// }
				/// ]]></example>
				Value							Invoke(const WString& name, collections::Array<Value>& arguments)const;

				/// <summary>Attach a callback function for the event.</summary>
				/// <returns>The event handler for this attachment. You need to keep it to detach the callback function.</returns>
				/// <param name="name">The registered name for the event.</param>
				/// <param name="function">The callback function.</param>
				/// <remarks>
				/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
				/// </remarks>
				/// <example><![CDATA[
				/// // reflectable C++ types
				/// 
				/// namespace mynamespace
				/// {
				///     class MyClass : public Object, public Description<MyClass>
				///     {
				///     private:
				///         WString prop;
				///     public:
				///         Event<void(const WString&, const WString&)> PropChanged;
				/// 
				///         WString GetProp()
				///         {
				///             return prop;
				///         }
				/// 
				///         void SetProp(const WString& value)
				///         {
				///             if (prop != value)
				///             {
				///                 auto old = prop;
				///                 prop = value;
				///                 PropChanged(old, prop);
				///             }
				///         }
				///     };
				/// }
				/// 
				/// #define MY_TYPELIST(F)\
				///     F(mynamespace::MyClass)\
				/// 
				/// // it is recommended to put the content below in a separated header file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             MY_TYPELIST(DECL_TYPE_INFO)
				///         }
				///     }
				/// }
				/// 
				/// // it is recommended to put the content below in a separated cpp file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             using namespace mynamespace;
				/// 
				/// #define _ ,
				/// 
				///             MY_TYPELIST(IMPL_CPP_TYPE_INFO)
				/// 
				///             BEGIN_CLASS_MEMBER(MyClass)
				///                 CLASS_MEMBER_CONSTRUCTOR(Ptr<MyClass>(), NO_PARAMETER)
				///                 CLASS_MEMBER_EVENT(PropChanged)
				///                 CLASS_MEMBER_PROPERTY_EVENT_FAST(Prop, PropChanged)
				///             END_CLASS_MEMBER(MyClass)
				/// 
				/// #undef  _
				///         }
				///     }
				/// }
				/// 
				/// class MyTypeLoader : public Object, public ITypeLoader
				/// {
				/// public:
				///     void Load(ITypeManager* manager)
				///     {
				///         MY_TYPELIST(ADD_TYPE_INFO)
				///     }
				/// 
				///     void Unload(ITypeManager* manager)
				///     {
				///     }
				/// };
				/// 
				/// // main function
				/// 
				/// int main()
				/// {
				///     LoadPredefinedTypes();
				///     GetGlobalTypeManager()->AddTypeLoader(new MyTypeLoader);
				///     GetGlobalTypeManager()->Load();
				///     {
				///         auto td = GetTypeDescriptor(L"mynamespace::MyClass");
				///         auto myClass = Value::Create(td);
				///         myClass.SetProperty(L"Prop", BoxValue<WString>(L"Zero"));
				/// 
				///         using CallbackType = Func<void(const WString&, const WString&)>;
				///         CallbackType callbackFunction = [](const WString& oldProp, const WString& newProp)
				///         {
				///             Console::WriteLine(L"myClass.Prop changed: " + oldProp + L" -> " + newProp);
				///         };
				///         auto handler = myClass.AttachEvent(L"PropChanged", BoxParameter<CallbackType>(callbackFunction));
				/// 
				///         myClass.SetProperty(L"Prop", BoxValue<WString>(L"One"));
				///         myClass.SetProperty(L"Prop", BoxValue<WString>(L"Two"));
				///         myClass.DetachEvent(L"PropChanged", handler);
				///         myClass.SetProperty(L"Prop", BoxValue<WString>(L"Three"));
				///     }
				///     DestroyGlobalTypeManager();
				/// }
				/// ]]></example>
				Ptr<IEventHandler>				AttachEvent(const WString& name, const Value& function)const;

				/// <summary>Detach a callback function from the event.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="name">The registered name for the event.</param>
				/// <param name="handler">The event handler returned from <see cref="AttachEvent"/>.</param>
				/// <remarks>
				/// <p>Only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is <b>off</b>.</p>
				/// </remarks>
				/// <example><![CDATA[
				/// // reflectable C++ types
				/// 
				/// namespace mynamespace
				/// {
				///     class MyClass : public Object, public Description<MyClass>
				///     {
				///     private:
				///         WString prop;
				///     public:
				///         Event<void(const WString&, const WString&)> PropChanged;
				/// 
				///         WString GetProp()
				///         {
				///             return prop;
				///         }
				/// 
				///         void SetProp(const WString& value)
				///         {
				///             if (prop != value)
				///             {
				///                 auto old = prop;
				///                 prop = value;
				///                 PropChanged(old, prop);
				///             }
				///         }
				///     };
				/// }
				/// 
				/// #define MY_TYPELIST(F)\
				///     F(mynamespace::MyClass)\
				/// 
				/// // it is recommended to put the content below in a separated header file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             MY_TYPELIST(DECL_TYPE_INFO)
				///         }
				///     }
				/// }
				/// 
				/// // it is recommended to put the content below in a separated cpp file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             using namespace mynamespace;
				/// 
				/// #define _ ,
				/// 
				///             MY_TYPELIST(IMPL_CPP_TYPE_INFO)
				/// 
				///             BEGIN_CLASS_MEMBER(MyClass)
				///                 CLASS_MEMBER_CONSTRUCTOR(Ptr<MyClass>(), NO_PARAMETER)
				///                 CLASS_MEMBER_EVENT(PropChanged)
				///                 CLASS_MEMBER_PROPERTY_EVENT_FAST(Prop, PropChanged)
				///             END_CLASS_MEMBER(MyClass)
				/// 
				/// #undef  _
				///         }
				///     }
				/// }
				/// 
				/// class MyTypeLoader : public Object, public ITypeLoader
				/// {
				/// public:
				///     void Load(ITypeManager* manager)
				///     {
				///         MY_TYPELIST(ADD_TYPE_INFO)
				///     }
				/// 
				///     void Unload(ITypeManager* manager)
				///     {
				///     }
				/// };
				/// 
				/// // main function
				/// 
				/// int main()
				/// {
				///     LoadPredefinedTypes();
				///     GetGlobalTypeManager()->AddTypeLoader(new MyTypeLoader);
				///     GetGlobalTypeManager()->Load();
				///     {
				///         auto td = GetTypeDescriptor(L"mynamespace::MyClass");
				///         auto myClass = Value::Create(td);
				///         myClass.SetProperty(L"Prop", BoxValue<WString>(L"Zero"));
				/// 
				///         using CallbackType = Func<void(const WString&, const WString&)>;
				///         CallbackType callbackFunction = [](const WString& oldProp, const WString& newProp)
				///         {
				///             Console::WriteLine(L"myClass.Prop changed: " + oldProp + L" -> " + newProp);
				///         };
				///         auto handler = myClass.AttachEvent(L"PropChanged", BoxParameter<CallbackType>(callbackFunction));
				/// 
				///         myClass.SetProperty(L"Prop", BoxValue<WString>(L"One"));
				///         myClass.SetProperty(L"Prop", BoxValue<WString>(L"Two"));
				///         myClass.DetachEvent(L"PropChanged", handler);
				///         myClass.SetProperty(L"Prop", BoxValue<WString>(L"Three"));
				///     }
				///     DestroyGlobalTypeManager();
				/// }
				/// ]]></example>
				bool							DetachEvent(const WString& name, Ptr<IEventHandler> handler)const;
#endif

				/// <summary>Dispose the object if <b>GetValueType()</b> returns <b>RawPtr</b>.</summary>
				/// <returns>
				/// Returns true if the object is disposed.
				/// Returns false if the object cannot be disposed.
				/// An exception will be thrown if the reference counter is not 0.
				///</returns>
				/// <example><![CDATA[
				/// // reflectable C++ types
				/// 
				/// namespace mynamespace
				/// {
				///     class SharedClass : public Object, public Description<SharedClass>
				///     {
				///     public:
				///         SharedClass()
				///         {
				///             Console::WriteLine(L"SharedClass::SharedClass()");
				///         }
				/// 
				///         ~SharedClass()
				///         {
				///             Console::WriteLine(L"SharedClass::~SharedClass()");
				///         }
				///     };
				/// 
				///     class RawClass : public Object, public Description<RawClass>
				///     {
				///     public:
				///         RawClass()
				///         {
				///             Console::WriteLine(L"RawClass::RawClass()");
				///         }
				/// 
				///         ~RawClass()
				///         {
				///             Console::WriteLine(L"RawClass::~RawClass()");
				///         }
				///     };
				/// }
				/// 
				/// #define MY_TYPELIST(F)\
				///     F(mynamespace::SharedClass)\
				///     F(mynamespace::RawClass)\
				/// 
				/// // it is recommended to put the content below in a separated header file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             MY_TYPELIST(DECL_TYPE_INFO)
				///         }
				///     }
				/// }
				/// 
				/// // it is recommended to put the content below in a separated cpp file
				/// 
				/// namespace vl
				/// {
				///     namespace reflection
				///     {
				///         namespace description
				///         {
				///             using namespace mynamespace;
				/// 
				/// #define _ ,
				/// 
				///             MY_TYPELIST(IMPL_CPP_TYPE_INFO)
				/// 
				///             BEGIN_CLASS_MEMBER(SharedClass)
				///                 CLASS_MEMBER_CONSTRUCTOR(Ptr<SharedClass>(), NO_PARAMETER)
				///             END_CLASS_MEMBER(SharedClass)
				/// 
				///             BEGIN_CLASS_MEMBER(RawClass)
				///                 CLASS_MEMBER_CONSTRUCTOR(RawClass*(), NO_PARAMETER)
				///             END_CLASS_MEMBER(RawClass)
				/// 
				/// #undef  _
				///         }
				///     }
				/// }
				/// 
				/// class MyTypeLoader : public Object, public ITypeLoader
				/// {
				/// public:
				///     void Load(ITypeManager* manager)
				///     {
				///         MY_TYPELIST(ADD_TYPE_INFO)
				///     }
				/// 
				///     void Unload(ITypeManager* manager)
				///     {
				///     }
				/// };
				/// 
				/// // main function
				/// 
				/// int main()
				/// {
				///     LoadPredefinedTypes();
				///     GetGlobalTypeManager()->AddTypeLoader(new MyTypeLoader);
				///     GetGlobalTypeManager()->Load();
				///     {
				///         auto sharedClass = Value::Create(L"mynamespace::SharedClass");
				///         auto rawClass = Value::Create(L"mynamespace::RawClass");
				/// 
				///         Console::WriteLine(L"sharedClass is " + WString(sharedClass.GetValueType() == Value::SharedPtr ? L"SharedPtr" : L"RawPtr"));
				///         Console::WriteLine(L"rawClass is " + WString(rawClass.GetValueType() == Value::SharedPtr ? L"SharedPtr" : L"RawPtr"));
				/// 
				///         rawClass.DeleteRawPtr();
				///     }
				///     DestroyGlobalTypeManager();
				/// }
				/// ]]></example>
				bool							DeleteRawPtr();
			};

/***********************************************************************
ValueType
***********************************************************************/

			class IValueType : public virtual IDescriptable, public Description<IValueType>
			{
			public:
				template<typename T>
				class TypedBox : public IBoxedValue
				{
				private:
					template<typename U = T>
					static CompareResult ComparePrimitiveInternal(const U& a, const U& b, typename AcceptAlways<vint, decltype(&TypedValueSerializerProvider<U>::Compare)>::Type)
					{
						return TypedValueSerializerProvider<U>::Compare(a, b);
					}

					template<typename U = T>
					static CompareResult ComparePrimitiveInternal(const U& a, const U& b, double)
					{
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdynamic-class-memaccess"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
						auto result = memcmp(&a, &b, sizeof(U));
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
						if (result < 0) return IBoxedValue::Smaller;
						if (result > 0) return IBoxedValue::Greater;
						return IBoxedValue::Equal;
					}
				public:
					T							value;

					TypedBox()
						:value{}
					{
					}

					TypedBox(const T& _value)
						:value(_value)
					{
					}

					Ptr<IBoxedValue> Copy()override
					{
						return new TypedBox<T>(value);
					}

					CompareResult ComparePrimitive(Ptr<IBoxedValue> boxedValue)override
					{
						if (auto typedBox = boxedValue.Cast<TypedBox<T>>())
						{
							return ComparePrimitiveInternal(value, typedBox->value, (vint)0);
						}
						else
						{
							return IBoxedValue::NotComparable;
						}
					}
				};

				virtual Value						CreateDefault() = 0;
				virtual IBoxedValue::CompareResult	Compare(const Value& a, const Value& b) = 0;
			};

			class IEnumType : public virtual IDescriptable, public Description<IEnumType>
			{
			public:
				virtual bool					IsFlagEnum() = 0;
				virtual vint					GetItemCount() = 0;
				virtual WString					GetItemName(vint index) = 0;
				virtual vuint64_t				GetItemValue(vint index) = 0;
				virtual vint					IndexOfItem(WString name) = 0;

				virtual Value					ToEnum(vuint64_t value) = 0;
				virtual vuint64_t				FromEnum(const Value& value) = 0;
			};

			class ISerializableType : public virtual IDescriptable, public Description<ISerializableType>
			{
			public:
				virtual bool					Serialize(const Value& input, WString& output) = 0;
				virtual bool					Deserialize(const WString& input, Value& output) = 0;
			};

/***********************************************************************
ITypeDescriptor (type)
***********************************************************************/

			enum class TypeInfoHint
			{
				Normal,
				LazyList,
				Array,
				List,
				SortedList,
				ObservableList,
				Dictionary,
				NativeCollectionReference,
			};

			class ITypeInfo : public virtual IDescriptable, public Description<ITypeInfo>
			{
			public:
				enum Decorator
				{
					RawPtr,
					SharedPtr,
					Nullable,
					TypeDescriptor,
					Generic,
				};

				virtual Decorator				GetDecorator() = 0;
				virtual TypeInfoHint			GetHint() = 0;
				virtual ITypeInfo*				GetElementType() = 0;
				virtual ITypeDescriptor*		GetTypeDescriptor() = 0;
				virtual vint					GetGenericArgumentCount() = 0;
				virtual ITypeInfo*				GetGenericArgument(vint index) = 0;
				virtual WString					GetTypeFriendlyName() = 0;
			};

/***********************************************************************
ITypeDescriptor (basic)
***********************************************************************/

			class IMemberInfo : public virtual IDescriptable, public Description<IMemberInfo>
			{
			public:
				virtual ITypeDescriptor*		GetOwnerTypeDescriptor()=0;
				virtual const WString&			GetName()=0;
			};

/***********************************************************************
ITypeDescriptor (event)
***********************************************************************/

			class IEventHandler : public virtual IDescriptable, public Description<IEventHandler>
			{
			public:
				virtual bool					IsAttached()=0;
			};

			class IEventInfo : public virtual IMemberInfo, public Description<IEventInfo>
			{
			public:
				class ICpp : public virtual IDescriptable, public Description<ICpp>
				{
				public:
					/*
					Arguments:
						$Name:					Event name
						$This:					Expression for the "this" argument
						$Handler:				Event handler function / Event handler object
						$Arguments:				Expressions for arguments separated by ", "
					Default (for Vlpp Event):
						Attach:					::vl::__vwsn::EventAttach($This->$Name, $Handler)
						Detach:					::vl::__vwsn::EventDetach($This->$Name, $Handler)
						Invoke:					::vl::__vwsn::EventInvoke($This->$Name)($Arguments)

					GetInvokeTemplate() == L"*":
						This event does not exist in C++
					*/
					virtual const WString&		GetAttachTemplate() = 0;
					virtual const WString&		GetDetachTemplate() = 0;
					virtual const WString&		GetInvokeTemplate() = 0;
				};
				/*
				Priority:
					1. Use ICpp
					2. Use Default
				*/
				virtual ICpp*					GetCpp() = 0;

				virtual ITypeInfo*				GetHandlerType()=0;
				virtual vint					GetObservingPropertyCount()=0;
				virtual IPropertyInfo*			GetObservingProperty(vint index)=0;
				virtual Ptr<IEventHandler>		Attach(const Value& thisObject, Ptr<IValueFunctionProxy> handler)=0;
				virtual bool					Detach(const Value& thisObject, Ptr<IEventHandler> handler)=0;
				virtual void					Invoke(const Value& thisObject, Ptr<IValueList> arguments)=0;
			};

/***********************************************************************
ITypeDescriptor (property)
***********************************************************************/

			class IPropertyInfo : public virtual IMemberInfo, public Description<IPropertyInfo>
			{
			public:
				class ICpp : public virtual IDescriptable, public Description<ICpp>
				{
				public:
					/*
					Arguments:
						$Type:					C++ full type name
						$Name:					Property name
						$This:					Expression for the "this" argument
					Default:
						Struct:					$This.$Name
						Class:					$This->$Name
					Example:
						Token in syntax tree:	$This->$Name.value

					GetReferenceTemplate() == L"*":
						This property does not exist in C++
					*/
					virtual const WString&		GetReferenceTemplate() = 0;
				};
				/*
				Priority:
					1. Use ICpp
					2. Use ICpp from getter and setter
					3. Use default
				*/
				virtual ICpp*					GetCpp() = 0;

				virtual bool					IsReadable()=0;
				virtual bool					IsWritable()=0;
				virtual ITypeInfo*				GetReturn()=0;
				virtual IMethodInfo*			GetGetter()=0;
				virtual IMethodInfo*			GetSetter()=0;
				virtual IEventInfo*				GetValueChangedEvent()=0;
				virtual Value					GetValue(const Value& thisObject)=0;
				virtual void					SetValue(Value& thisObject, const Value& newValue)=0;
			};

/***********************************************************************
ITypeDescriptor (method)
***********************************************************************/

			class IParameterInfo : public virtual IMemberInfo, public Description<IParameterInfo>
			{
			public:
				virtual ITypeInfo*				GetType()=0;
				virtual IMethodInfo*			GetOwnerMethod()=0;
			};

			class IMethodInfo : public virtual IMemberInfo, public Description<IMethodInfo>
			{
			public:
				class ICpp : public virtual IDescriptable, public Description<ICpp>
				{
				public:
					/*
					Arguments:
						$Type:					C++ full type name
						$Func:					C++ function type (e.g. void(int)), object type not included for method
						$Name:					Method name
						$This:					Expression for the "this" argument;
						$Arguments:				Expressions for arguments separated by ", "
					Default:
						Constructor:			new $Type($Arguments)
						Static:					$Type::$Name($Arguments)
						Normal:					$This->$Name($Arguments)
					Example:
						External constructor:	<full-function-name>($Arguments)
						External method:		<full-function-name>($This, $Arguments)
						Renamed method:			$This-><function-name>($Arguments)

					GetInvokeTemplate() == L"*":
						This method does not exist in C++
					*/
					virtual const WString&		GetInvokeTemplate() = 0;
					virtual const WString&		GetClosureTemplate() = 0;
				};
				/*
				Priority:
					1. Use ICpp
					2. Use default
				*/
				virtual ICpp*					GetCpp() = 0;

				virtual IMethodGroupInfo*		GetOwnerMethodGroup()=0;
				virtual IPropertyInfo*			GetOwnerProperty()=0;
				virtual vint					GetParameterCount()=0;
				virtual IParameterInfo*			GetParameter(vint index)=0;
				virtual ITypeInfo*				GetReturn()=0;
				virtual bool					IsStatic()=0;
				virtual void					CheckArguments(collections::Array<Value>& arguments)=0;
				virtual Value					Invoke(const Value& thisObject, collections::Array<Value>& arguments)=0;
				virtual Value					CreateFunctionProxy(const Value& thisObject) = 0;
			};

			class IMethodGroupInfo : public virtual IMemberInfo, public Description<IMethodGroupInfo>
			{
			public:
				virtual vint					GetMethodCount()=0;
				virtual IMethodInfo*			GetMethod(vint index)=0;
			};

/***********************************************************************
ITypeDescriptor
***********************************************************************/

			enum class TypeDescriptorFlags : vint
			{
				Undefined			= 0,
				Object				= 1<<0,
				IDescriptable		= 1<<1,
				Class				= 1<<2,
				Interface			= 1<<3,
				Primitive			= 1<<4,
				Struct				= 1<<5,
				FlagEnum			= 1<<6,
				NormalEnum			= 1<<7,

				ClassType			= Object | Class,
				InterfaceType		= IDescriptable | Interface,
				ReferenceType		= ClassType | InterfaceType,
				EnumType			= FlagEnum | NormalEnum,
				StructType			= Primitive | Struct,
			};

			inline TypeDescriptorFlags operator&(TypeDescriptorFlags a, TypeDescriptorFlags b)
			{
				return (TypeDescriptorFlags)((vint)a & (vint)b);
			}

			inline TypeDescriptorFlags operator|(TypeDescriptorFlags a, TypeDescriptorFlags b)
			{
				return (TypeDescriptorFlags)((vint)a | (vint)b);
			}

			/// <summary>Metadata class for reflectable types.</summary>
			class ITypeDescriptor : public virtual IDescriptable, public Description<ITypeDescriptor>
			{
			public:
				class ICpp : public virtual IDescriptable, public Description<ICpp>
				{
				public:
					/*
						Default:				refer to TypeInfoContent::VlppType

						GetFullName() == L"*":
							This type does not exist in C++
					*/
					virtual const WString&		GetFullName() = 0;
				};
				/*
				Priority:
					1. Use ICpp
					2. Use default
				*/
				virtual ICpp*					GetCpp() = 0;

				virtual TypeDescriptorFlags		GetTypeDescriptorFlags() = 0;
				virtual bool					IsAggregatable() = 0;
				virtual const WString&			GetTypeName() = 0;

				virtual IValueType*				GetValueType() = 0;
				virtual IEnumType*				GetEnumType() = 0;
				virtual ISerializableType*		GetSerializableType() = 0;

				virtual vint					GetBaseTypeDescriptorCount() = 0;
				virtual ITypeDescriptor*		GetBaseTypeDescriptor(vint index) = 0;
				virtual bool					CanConvertTo(ITypeDescriptor* targetType) = 0;

				virtual vint					GetPropertyCount() = 0;
				virtual IPropertyInfo*			GetProperty(vint index) = 0;
				virtual bool					IsPropertyExists(const WString& name, bool inheritable) = 0;
				virtual IPropertyInfo*			GetPropertyByName(const WString& name, bool inheritable) = 0;

				virtual vint					GetEventCount() = 0;
				virtual IEventInfo*				GetEvent(vint index) = 0;
				virtual bool					IsEventExists(const WString& name, bool inheritable) = 0;
				virtual IEventInfo*				GetEventByName(const WString& name, bool inheritable) = 0;

				virtual vint					GetMethodGroupCount() = 0;
				virtual IMethodGroupInfo*		GetMethodGroup(vint index) = 0;
				virtual bool					IsMethodGroupExists(const WString& name, bool inheritable) = 0;
				virtual IMethodGroupInfo*		GetMethodGroupByName(const WString& name, bool inheritable) = 0;
				virtual IMethodGroupInfo*		GetConstructorGroup() = 0;
			};

#ifndef VCZH_DEBUG_NO_REFLECTION

/***********************************************************************
ITypeManager
***********************************************************************/

			class ITypeManager;

			/// <summary>Delay loading for registering reflectable types.</summary>
			class ITypeLoader : public virtual Interface
			{
			public:
				/// <summary>Called when it is time to register types.</summary>
				/// <param name="manager">The type manager.</param>
				virtual void					Load(ITypeManager* manager)=0;

				/// <summary>Called when it is time to unregister types.</summary>
				/// <param name="manager">The type manager.</param>
				/// <remarks>
				/// Types cannot be unregistered one by one,
				/// they are removed at the same time by calling
				/// [F:vl.reflection.description.DestroyGlobalTypeManager] or
				/// [F:vl.reflection.description.ResetGlobalTypeManager].
				/// Here is just a chance for reverse extra steps, when these steps are taken in <see cref="Load"/>.
				/// </remarks>
				virtual void					Unload(ITypeManager* manager)=0;
			};

			/// <summary>A type manager to access all reflectable types.</summary>
			class ITypeManager : public virtual Interface
			{
			public:
				/// <summary>Get the number of all registered types.</summary>
				/// <returns>The number of all registered types.</returns>
				virtual vint					GetTypeDescriptorCount()=0;

				/// <summary>Get one registered type.</summary>
				/// <returns>A registered type specified by the index.</returns>
				/// <param name="index">The index for retriving the registered type.</param>
				/// <remarks>
				/// The index itself does not has any specific meaning.
				/// And it is no guarantee that an index will always return the same type for each execution of the same process.
				/// </remarks>
				virtual ITypeDescriptor*		GetTypeDescriptor(vint index)=0;
				virtual ITypeDescriptor*		GetTypeDescriptor(const WString& name)=0;
				virtual bool					SetTypeDescriptor(const WString& name, Ptr<ITypeDescriptor> typeDescriptor)=0;

				/// <summary>Delay register some types.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="typeLoader">A type loader for delay registering.</param>
				/// <remarks>
				/// You can still call this function after <see cref="Load"/> is called.
				/// In this case, there is no delay registering, all types in this loader will be registered immediately.
				/// </remarks>
				virtual bool					AddTypeLoader(Ptr<ITypeLoader> typeLoader)=0;
				virtual bool					RemoveTypeLoader(Ptr<ITypeLoader> typeLoader)=0;

				/// <summary>Load all added type loaders.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				virtual bool					Load()=0;
				virtual bool					Unload()=0;
				virtual bool					Reload()=0;
				virtual bool					IsLoaded()=0;
				virtual ITypeDescriptor*		GetRootType()=0;
			};

			/// <summary>Get the type manager.</summary>
			/// <returns>Returns the type manager.</returns>
			extern ITypeManager*				GetGlobalTypeManager();

			/// <summary>Unload all types and free the type manager.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <remarks>
			/// After calling this function, you can no longer register new types,
			/// and calling <see cref="GetGlobalTypeManager"/> will always get null.
			/// </remarks>

			extern bool							DestroyGlobalTypeManager();

			/// <summary>Unload all types and reset the type manager.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			/// <remarks>
			/// <p>
			/// This function is similar to <see cref="DestroyGlobalTypeManager"/>,
			/// but calling this function allows types to be registsred again.
			/// </p>
			/// <p>
			/// This function is very useful for unit testing.
			/// In each test case, you can first register all types,
			/// and after the test case is finished, call this function to reset all types.
			/// You can do this again and again in the other test cases,
			/// so that these test cases don't affect each other.
			/// </p>
			/// </remarks>
			extern bool							ResetGlobalTypeManager();

			/// <summary>Get a registered type given the registered name.</summary>
			/// <returns>Returns the metadata class for this registered type.</returns>
			/// <param name="name">
			/// The registered name.
			/// Note that this is not the full name of the C++ type,
			/// it is the name what is used to registere this type.</param>
			/// <remarks>
			/// Returning null means the type registration is declared but the type manager has not started.
			/// </remarks>
			extern ITypeDescriptor*				GetTypeDescriptor(const WString& name);
			extern bool							IsInterfaceType(ITypeDescriptor* typeDescriptor, bool& acceptProxy);
			extern void							LogTypeManager(stream::TextWriter& writer);
			extern void							GenerateMetaonlyTypes(stream::IStream& outputStream);
			extern void							LoadMetaonlyTypes(stream::IStream& inputStream);

/***********************************************************************
Cpp Helper Functions
***********************************************************************/

			extern WString						CppGetFullName(ITypeDescriptor* type);
			extern WString						CppGetReferenceTemplate(IPropertyInfo* prop);
			extern WString						CppGetClosureTemplate(IMethodInfo* method);
			extern WString						CppGetInvokeTemplate(IMethodInfo* method);
			extern WString						CppGetAttachTemplate(IEventInfo* ev);
			extern WString						CppGetDetachTemplate(IEventInfo* ev);
			extern WString						CppGetInvokeTemplate(IEventInfo* ev);

			extern bool							CppExists(ITypeDescriptor* type);
			extern bool							CppExists(IPropertyInfo* prop);
			extern bool							CppExists(IMethodInfo* method);
			extern bool							CppExists(IEventInfo* ev);

#endif

/***********************************************************************
Exceptions
***********************************************************************/

			class TypeDescriptorException abstract : public Exception
			{
			public:
				TypeDescriptorException(const WString& message)
					:Exception(message)
				{
				}
			};

			class ValueNotDisposableException : public TypeDescriptorException
			{
			public:
				ValueNotDisposableException()
					:TypeDescriptorException(L"Cannot dispose an object whose reference counter is not 0.")
				{
				}
			};

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

			class TypeNotExistsException : public TypeDescriptorException
			{
			public:
				TypeNotExistsException(const WString& name)
					:TypeDescriptorException(L"Cannot find the type \""+name+L"\".")
				{
				}
			};

			class ConstructorNotExistsException : public TypeDescriptorException
			{
			public:
				ConstructorNotExistsException(ITypeDescriptor* type)
					:TypeDescriptorException(L"Cannot find any constructor in type \"" + type->GetTypeName() + L"\".")
				{
				}
			};

			class MemberNotExistsException : public TypeDescriptorException
			{
			public:
				MemberNotExistsException(const WString& name, ITypeDescriptor* type)
					:TypeDescriptorException(L"Cannot find the member \"" + name + L"\" in type \"" + type->GetTypeName() + L"\".")
				{
				}
			};

			class PropertyIsNotReadableException : public TypeDescriptorException
			{
			public:
				PropertyIsNotReadableException(IPropertyInfo* propertyInfo)
					:TypeDescriptorException(L"Cannot read value from a property \"" + propertyInfo->GetName() + L"\" that is not readable in type \"" + propertyInfo->GetOwnerTypeDescriptor()->GetTypeName() + L"\".")
				{
				}
			};

			class PropertyIsNotWritableException : public TypeDescriptorException
			{
			public:
				PropertyIsNotWritableException(IPropertyInfo* propertyInfo)
					:TypeDescriptorException(L"Cannot write value to a property \"" + propertyInfo->GetName() + L"\" that is not writable in type \"" + propertyInfo->GetOwnerTypeDescriptor()->GetTypeName() + L"\".")
				{
				}
			};

			class ArgumentNullException : public TypeDescriptorException
			{
			public:
				ArgumentNullException(const WString& name, const WString& member)
					:TypeDescriptorException(L"Argument \"" + name + L"\" cannot be null when accessing its member \"" + member + L"\".")
				{
				}

				ArgumentNullException(const WString& name, IMethodInfo* target)
					:TypeDescriptorException(L"Argument \"" + name + L"\" cannot be null when invoking method \"" + target->GetName() + L"\" in type \"" + target->GetOwnerTypeDescriptor()->GetTypeName() + L"\".")
				{
				}

				ArgumentNullException(const WString& name, IEventInfo* target)
					:TypeDescriptorException(L"Argument \"" + name + L"\" cannot be null when accessing event \"" + target->GetName() + L"\" in type \"" + target->GetOwnerTypeDescriptor()->GetTypeName() + L"\".")
				{
				}

				ArgumentNullException(const WString& name, IPropertyInfo* target)
					:TypeDescriptorException(L"Argument \"" + name + L"\" cannot be null when invoking property \"" + target->GetName() + L"\" in type \"" + target->GetOwnerTypeDescriptor()->GetTypeName() + L"\".")
				{
				}
			};

			class ArgumentTypeMismtatchException : public TypeDescriptorException
			{
			public:
				ArgumentTypeMismtatchException(const WString& name, ITypeInfo* expected, const Value& actual)
					:TypeDescriptorException(L"Argument \"" + name + L"\" cannot convert from \"" + actual.GetTypeFriendlyName() + L"\" to \"" + expected->GetTypeFriendlyName() + L"\".")
				{
				}

				ArgumentTypeMismtatchException(const WString& name, ITypeDescriptor* type, Value::ValueType valueType, const Value& actual)
					:TypeDescriptorException(L"Argument \"" + name + L"\" cannot convert from \"" + actual.GetTypeFriendlyName() + L"\" to \"" +
						(valueType == Value::SharedPtr ? L"Ptr<" : L"") + type->GetTypeName() + (valueType == Value::SharedPtr ? L">" : valueType == Value::RawPtr ? L"*" : L"")
						+ L"\".")
				{
				}
			};

			class ArgumentCountMismtatchException : public TypeDescriptorException
			{
			public:
				ArgumentCountMismtatchException()
					:TypeDescriptorException(L"Argument count does not match the definition.")
				{
				}

				ArgumentCountMismtatchException(IMethodGroupInfo* target)
					:TypeDescriptorException(L"Argument count does not match the definition when invoking method \"" + target->GetName() + L"\" in type \"" + target->GetOwnerTypeDescriptor()->GetTypeName() + L"\".")
				{
				}
			};

#endif
		}
	}
}

#endif