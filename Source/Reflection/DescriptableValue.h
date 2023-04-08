/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_DESCRIPTABLEVALUE
#define VCZH_REFLECTION_DESCRIPTABLEVALUE

#include "DescriptableObject.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			class ITypeInfo;
			class IMethodGroupInfo;
			class IMethodInfo;
			class IPropertyInfo;
			class IEventInfo;
			class IEventHandler;
			class IValueFunctionProxy;
		}

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
			public:
				/// <summary>Create a null value.</summary>
				Value();
				Value(const Value& value);
				Value&							operator=(const Value& value);

				friend std::strong_ordering		operator<=>(const Value& a, const Value& b);
				friend bool						operator==(const Value& a, const Value& b) { return (a <=> b) == 0; }

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
				///         auto handler = myClass.AttachEvent(L"PropChanged", BoxParameter(callbackFunction));
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
				///         auto handler = myClass.AttachEvent(L"PropChanged", BoxParameter(callbackFunction));
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
		}
	}
}

#endif