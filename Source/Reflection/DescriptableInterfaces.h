/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_DESCRIPTABLEINTERFACES
#define VCZH_REFLECTION_DESCRIPTABLEINTERFACES

#include "DescriptableValue.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			class IValueReadonlyList;

			template<typename T>
			struct TypedValueSerializerProvider;
		}

		namespace description
		{
/***********************************************************************
ValueType
***********************************************************************/

			namespace pbt_selector
			{
				template<PredefinedBoxableType _Value>
				struct SelectorBase { static constexpr PredefinedBoxableType Value = _Value; };

				template<typename T> struct Selector : SelectorBase<PredefinedBoxableType::PBT_Unknown> {};

				template<> struct Selector<vint8_t> : SelectorBase<PredefinedBoxableType::PBT_S8> {};
				template<> struct Selector<vint16_t> : SelectorBase<PredefinedBoxableType::PBT_S16> {};
				template<> struct Selector<vint32_t> : SelectorBase<PredefinedBoxableType::PBT_S32> {};
				template<> struct Selector<vint64_t> : SelectorBase<PredefinedBoxableType::PBT_S64> {};

				template<> struct Selector<vuint8_t> : SelectorBase<PredefinedBoxableType::PBT_U8> {};
				template<> struct Selector<vuint16_t> : SelectorBase<PredefinedBoxableType::PBT_U16> {};
				template<> struct Selector<vuint32_t> : SelectorBase<PredefinedBoxableType::PBT_U32> {};
				template<> struct Selector<vuint64_t> : SelectorBase<PredefinedBoxableType::PBT_U64> {};

				template<> struct Selector<float> : SelectorBase<PredefinedBoxableType::PBT_F8> {};
				template<> struct Selector<double> : SelectorBase<PredefinedBoxableType::PBT_F16> {};

				template<> struct Selector<bool> : SelectorBase<PredefinedBoxableType::PBT_BOOL> {};
				template<> struct Selector<wchar_t> : SelectorBase<PredefinedBoxableType::PBT_WCHAR> {};
				template<> struct Selector<WString> : SelectorBase<PredefinedBoxableType::PBT_STRING> {};
				template<> struct Selector<Locale> : SelectorBase<PredefinedBoxableType::PBT_LOCALE> {};
				template<> struct Selector<DateTime> : SelectorBase<PredefinedBoxableType::PBT_DATETIME> {};
			}

			class IValueType : public virtual IDescriptable, public Description<IValueType>
			{
			public:
				template<typename T>
				class TypedBox : public IBoxedValue
				{
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

					PredefinedBoxableType GetBoxableType()override
					{
						return pbt_selector::Selector<T>::Value;
					}

					Ptr<IBoxedValue> Copy()override
					{
						return Ptr(new TypedBox<T>(value));
					}

					CompareResult ComparePrimitive(Ptr<IBoxedValue> boxedValue)override
					{
						if (auto typedBox = boxedValue.Cast<TypedBox<T>>())
						{
							if constexpr (std::three_way_comparable<T, std::strong_ordering>)
							{
								auto r = value <=> typedBox->value;
								if (r < 0) return IBoxedValue::Smaller;
								if (r > 0) return IBoxedValue::Greater;
								return IBoxedValue::Equal;
							}
							else if constexpr (std::three_way_comparable<T, std::partial_ordering>)
							{
								auto r = value <=> typedBox->value;
								if (r == std::partial_ordering::unordered) return IBoxedValue::NotComparable;
								if (r < 0) return IBoxedValue::Smaller;
								if (r > 0) return IBoxedValue::Greater;
								return IBoxedValue::Equal;
							}
						}
						return IBoxedValue::NotComparable;
					}
				};

				virtual Value						CreateDefault() = 0;
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
				virtual void					Invoke(const Value& thisObject, Ptr<IValueReadonlyList> arguments)=0;
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
				virtual vint					GetTypeVersion()=0;
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
			extern Ptr<ITypeLoader>				LoadMetaonlyTypes(stream::IStream& inputStream, const collections::Dictionary<WString, Ptr<ISerializableType>>& serializableTypes);

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
				ArgumentTypeMismtatchException(const WString& name, ITypeDescriptor* expected, ITypeDescriptor* actual)
					:TypeDescriptorException(L"Argument \"" + name + L"\" cannot convert from \"" + actual->GetTypeName() + L"\" to \"" + expected->GetTypeName() + L"\".")
				{
				}

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