/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_GUITYPEDESCRIPTORPREDEFINED
#define VCZH_REFLECTION_GUITYPEDESCRIPTORPREDEFINED

#include <math.h>
#include "GuiTypeDescriptor.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			struct VoidValue {};

/***********************************************************************
Collections
***********************************************************************/

			/// <summary>The reflectable version of <see cref="collections::IEnumerator`1"/>.</summary>
			class IValueEnumerator : public virtual IDescriptable, public Description<IValueEnumerator>
			{
			public:
				/// <summary>Get the reference to the current value in the enumerator.</summary>
				/// <returns>The current value.</returns>
				/// <remarks><see cref="Next"/> needs to be called to make the first value available.</remarks>
				virtual Value					GetCurrent() = 0;

				/// <summary>Get the position of the current value in the enumerator.</summary>
				/// <returns>The position of the current value.</returns>
				virtual vint					GetIndex() = 0;

				/// <summary>Prepare for the next value.</summary>
				/// <returns>Returns false if there is no more value.</returns>
				virtual bool					Next() = 0;
			};

			/// <summary>The reflectable version of <see cref="collections::IEnumerable`1"/>.</summary>
			/// <remarks><see cref="BoxParameter`1"/> will create a <see cref="Value"/> storing a shared pointer to an instance of this interface from an enumerable.</remarks>
			class IValueEnumerable : public virtual IDescriptable, public Description<IValueEnumerable>
			{
			public:
				/// <summary>
				/// Create an enumerator. <see cref="IValueEnumerator::Next"/> should be called before reading the first value.
				/// </summary>
				/// <returns>The enumerator.</returns>
				virtual Ptr<IValueEnumerator>	CreateEnumerator() = 0;

				/// <summary>Create an enumerable from another lazy list.</summary>
				/// <returns>The created enumerable.</returns>
				/// <param name="values">The lazy list to wrap.</param>
				static Ptr<IValueEnumerable>	Create(collections::LazyList<Value> values);
			};

			/// <summary>
			/// The reflectable version of readonly
			/// <see cref="collections::Array`2"/>,
			/// <see cref="collections::List`2"/> or
			/// <see cref="collections::SortedList`2"/>
			/// </summary>
			class IValueReadonlyList : public virtual IValueEnumerable, public Description<IValueReadonlyList>
			{
			public:
				/// <summary>Get the number of elements in the container.</summary>
				/// <returns>The number of elements.</returns>
				virtual vint					GetCount() = 0;

				/// <summary>Get the reference to the specified element.</summary>
				/// <returns>The reference to the specified element. It will crash when the index is out of range.</returns>
				/// <param name="index">The index of the element.</param>
				virtual Value					Get(vint index) = 0;

				/// <summary>Test does the list contain a value or not.</summary>
				/// <returns>Returns true if the list contains the specified value.</returns>
				/// <param name="value">The value to test.</param>
				virtual bool					Contains(const Value& value) = 0;

				/// <summary>Get the position of a value in this list.</summary>
				/// <returns>Returns the position of first element that equals to the specified value. Returns -1 if failed to find.</returns>
				/// <param name="value">The value to find.</param>
				virtual vint					IndexOf(const Value& value) = 0;
			};

			/// <summary>
			/// The reflectable version of readonly
			/// <see cref="collections::Array`2"/> or
			/// <see cref="collections::List`2"/>
			/// </summary>

			/// <remarks><see cref="BoxParameter`1"/> will create a <see cref="Value"/> storing a shared pointer to an instance of this interface from a container.</remarks>
			class IValueList : public virtual IValueReadonlyList, public Description<IValueList>
			{
			public:
				/// <summary>Replace an element in the specified position.</summary>
				/// <returns>Returns true if this operation succeeded. It will crash when the index is out of range</returns>
				/// <param name="index">The position of the element to replace.</param>
				/// <param name="value">The new value to replace.</param>
				virtual void					Set(vint index, const Value& value) = 0;

				/// <summary>Append a value at the end of the list.</summary>
				/// <returns>The index of the added item.</returns>
				/// <param name="value">The value to add.</param>
				virtual vint					Add(const Value& value) = 0;

				/// <summary>Insert a value at the specified position.</summary>
				/// <returns>The index of the added item. It will crash if the index is out of range</returns>
				/// <param name="index">The position to insert the value.</param>
				/// <param name="value">The value to add.</param>
				virtual vint					Insert(vint index, const Value& value) = 0;

				/// <summary>Remove an element from the list. If multiple elements equal to the specified value, only the first one will be removed.</summary>
				/// <returns>Returns true if the element is removed.</returns>
				/// <param name="value">The item to remove.</param>
				virtual bool					Remove(const Value& value) = 0;

				/// <summary>Remove an element at a specified position.</summary>
				/// <returns>Returns true if the element is removed. It will crash when the index is out of range.</returns>
				/// <param name="index">The index of the element to remove.</param>
				virtual bool					RemoveAt(vint index) = 0;

				/// <summary>Remove all elements.</summary>
				virtual void					Clear() = 0;

				/// <summary>Create an empty list.</summary>
				/// <returns>The created list.</returns>
				static Ptr<IValueList>			Create();

				/// <summary>Create a list with elements copied from another readonly list.</summary>
				/// <returns>The created list.</returns>
				/// <param name="values">Elements to copy.</param>
				static Ptr<IValueList>			Create(Ptr<IValueReadonlyList> values);

				/// <summary>Create a list with elements copied from another lazy list.</summary>
				/// <returns>The created list.</returns>
				/// <param name="values">Elements to copy.</param>
				static Ptr<IValueList>			Create(collections::LazyList<Value> values);
			};

			/// <summary>
			/// The reflectable version of list container which triggers an event whenever items are changed.
			/// </summary>
			class IValueObservableList : public virtual IValueList, public Description<IValueObservableList>
			{
				typedef void ItemChangedProc(vint index, vint oldCount, vint newCount);
			public:
				/// <summary>
				/// <p>Event that is triggered whenever items are changed.</p>
				/// <p>The first argument is the index of the first item that is changed.</p>
				/// <p>The second argument is the number of original items that are replaced by new items.</p>
				/// <p>The third argument is the number of new items that replace original items.</p>
				/// </summary>
				/// <remarks>
				/// <p>If an item is changed, oldCount and newCount are both 1.</p>
				/// <p>If several items are removed from the list, newCount is 0.</p>
				/// <p>If several items are inserted to the list, oldCount is 0.</p>
				/// <p>This event is triggered when the updating is done, original items are not possible to access at the moment.</p>
				/// </remarks>
				Event<ItemChangedProc>			ItemChanged;

				/// <summary>Create an empty list.</summary>
				/// <returns>The created list.</returns>
				static Ptr<IValueObservableList>	Create();

				/// <summary>Create a list with elements copied from another readonly list.</summary>
				/// <returns>The created list.</returns>
				/// <param name="values">Elements to copy.</param>
				static Ptr<IValueObservableList>	Create(Ptr<IValueReadonlyList> values);

				/// <summary>Create a list with elements copied from another lazy list.</summary>
				/// <returns>The created list.</returns>
				/// <param name="values">Elements to copy.</param>
				static Ptr<IValueObservableList>	Create(collections::LazyList<Value> values);
			};

			/// <summary>
			/// The reflectable version of readonly <see cref="collections::Dictionary`4"/>.
			/// </summary>
			class IValueReadonlyDictionary : public virtual IDescriptable, public Description<IValueReadonlyDictionary>
			{
			public:
				/// <summary>Get all keys.</summary>
				/// <returns>All keys.</returns>
				virtual Ptr<IValueReadonlyList>	GetKeys() = 0;

				/// <summary>Get all values.</summary>
				/// <returns>All values.</returns>
				virtual Ptr<IValueReadonlyList>	GetValues() = 0;

				/// <summary>Get the number of keys.</summary>
				/// <returns>The number of keys. It is also the number of values.</returns>
				virtual vint					GetCount() = 0;

				/// <summary>Get the value associated to a specified key.</summary>
				/// <returns>The reference to the value. It will crash if the key does not exist.</returns>
				/// <param name="key">The key to find.</param>
				virtual Value					Get(const Value& key) = 0;
			};

			/// <summary>
			/// The reflectable version of <see cref="collections::Dictionary`4"/>.
			/// </summary>
			/// <remarks><see cref="BoxParameter`1"/> will create a <see cref="Value"/> storing a shared pointer to an instance of this interface from a dictionary.</remarks>
			class IValueDictionary : public virtual IValueReadonlyDictionary, public Description<IValueDictionary>
			{
			public:
				/// <summary>Replace the value associated to a specified key.</summary>
				/// <returns>Returns true if the value is replaced.</returns>
				/// <param name="key">The key to find. If the key does not exist, it will be added to the dictionary.</param>
				/// <param name="value">The associated value to replace.</param>
				virtual void					Set(const Value& key, const Value& value) = 0;

				/// <summary>Remove a key with the associated value.</summary>
				/// <returns>Returns true if the key and the value is removed.</returns>
				/// <param name="key">The key to find.</param>
				virtual bool					Remove(const Value& key) = 0;

				/// <summary>Remove all elements.</summary>
				virtual void					Clear() = 0;

				/// <summary>Create an empty dictionary.</summary>
				/// <returns>The created dictionary.</returns>
				static Ptr<IValueDictionary>	Create();

				/// <summary>Create a dictionary with elements copied from another readonly dictionary.</summary>
				/// <returns>The created dictionary.</returns>
				/// <param name="values">Elements to copy.</param>
				static Ptr<IValueDictionary>	Create(Ptr<IValueReadonlyDictionary> values);

				/// <summary>Create a dictionary with elements copied from another lazy list.</summary>
				/// <returns>The created dictionary.</returns>
				/// <param name="values">Elements to copy.</param>
				static Ptr<IValueDictionary>	Create(collections::LazyList<collections::Pair<Value, Value>> values);
			};

/***********************************************************************
Interface Implementation Proxy
***********************************************************************/

			class IValueInterfaceProxy : public virtual IDescriptable, public Description<IValueInterfaceProxy>
			{
			public:
				virtual Value					Invoke(IMethodInfo* methodInfo, Ptr<IValueList> arguments) = 0;
			};

			class IValueFunctionProxy : public virtual IDescriptable, public Description<IValueFunctionProxy>
			{
			public:
				virtual Value					Invoke(Ptr<IValueList> arguments) = 0;
			};

			class IValueSubscription : public virtual IDescriptable, public Description<IValueSubscription>
			{
				typedef void ValueChangedProc(const Value& newValue);
			public:
				Event<ValueChangedProc>			ValueChanged;

				virtual bool					Open() = 0;
				virtual bool					Update() = 0;
				virtual bool					Close() = 0;
			};

/***********************************************************************
Interface Implementation Proxy (Implement)
***********************************************************************/

			class ValueInterfaceRoot : public virtual IDescriptable
			{
			protected:
				Ptr<IValueInterfaceProxy>		proxy;

				void SetProxy(Ptr<IValueInterfaceProxy> value)
				{
					proxy = value;
				}
			public:
				Ptr<IValueInterfaceProxy> GetProxy()
				{
					return proxy;
				}
			};

			template<typename T>
			class ValueInterfaceProxy
			{
			};

#pragma warning(push)
#pragma warning(disable:4250)
			template<typename TInterface, typename ...TBaseInterfaces>
			class ValueInterfaceImpl : public virtual ValueInterfaceRoot, public virtual TInterface, public ValueInterfaceProxy<TBaseInterfaces>...
			{
			public:
				~ValueInterfaceImpl()
				{
					FinalizeAggregation();
				}
			};
#pragma warning(pop)

/***********************************************************************
Runtime Exception
***********************************************************************/

			class IValueCallStack : public virtual IDescriptable, public Description<IValueCallStack>
			{
			public:
				virtual Ptr<IValueReadonlyDictionary>	GetLocalVariables() = 0;
				virtual Ptr<IValueReadonlyDictionary>	GetLocalArguments() = 0;
				virtual Ptr<IValueReadonlyDictionary>	GetCapturedVariables() = 0;
				virtual Ptr<IValueReadonlyDictionary>	GetGlobalVariables() = 0;
				virtual WString							GetFunctionName() = 0;
				virtual WString							GetSourceCodeBeforeCodegen() = 0;
				virtual WString							GetSourceCodeAfterCodegen() = 0;
				virtual vint							GetRowBeforeCodegen() = 0;
				virtual vint							GetRowAfterCodegen() = 0;
			};

			class IValueException : public virtual IDescriptable, public Description<IValueException>
			{
			public:
#pragma push_macro("GetMessage")
#if defined GetMessage
#undef GetMessage
#endif
				virtual WString							GetMessage() = 0;
#pragma pop_macro("GetMessage")
				virtual bool							GetFatal() = 0;
				virtual Ptr<IValueReadonlyList>			GetCallStack() = 0;

				static Ptr<IValueException>				Create(const WString& message);
			};
		}
	}
}

#endif
