/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_DESCRIPTABLEOBJECT
#define VCZH_REFLECTION_DESCRIPTABLEOBJECT

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
		namespace description
		{
			class ITypeDescriptor;
		}

/***********************************************************************
DescriptableObject
***********************************************************************/

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
		///     auto myClass = Ptr(new MyClass);
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
			atomic_vint								referenceCounter;

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

			// all fields are describing the object, it would be incorrect if they are copied from one to another.
			DescriptableObject(const DescriptableObject&) : DescriptableObject() {}
			DescriptableObject(DescriptableObject&&) : DescriptableObject() {}
			DescriptableObject& operator=(const DescriptableObject&) { return *this; }
			DescriptableObject& operator=(DescriptableObject&&) { return *this; }

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

			static void SetAssociatedTypeDescriptor(description::ITypeDescriptor* typeDescroptor)
			{
				associatedTypeDescriptor=typeDescroptor;
			}
#endif
		};

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
		template<typename T>
		description::ITypeDescriptor* Description<T>::associatedTypeDescriptor = 0;
#endif

/***********************************************************************
AggregatableDescription
***********************************************************************/

		/// <summary>
		/// Inherit from this class when you want to create a reflectable class that can be inherited by Workflow script classes.
		/// </summary>
		/// <typeparam name="T">Type that inherit this class.</typeparam>
		template<typename T>
		class AggregatableDescription : public Description<T>
		{
		};

/***********************************************************************
IDescriptable
***********************************************************************/

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
	struct ReferenceCounterOperator<T, std::enable_if_t<std::is_convertible_v<T*, reflection::DescriptableObject*>>>
	{
		static __forceinline atomic_vint* CreateCounter(T* reference)
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

		static __forceinline void DeleteReference(atomic_vint* counter, void* reference)
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

			class ObjectDisposedException : public TypeDescriptorException
			{
			public:
				ObjectDisposedException()
					:TypeDescriptorException(L"The referenced native object has been disposed.")
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
		}
	}
}

#endif