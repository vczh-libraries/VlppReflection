/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_METADATA_METADATA
#define VCZH_REFLECTION_METADATA_METADATA

#include "../Predefined/ObservableList.h"
#include "../Predefined/PredefinedTypes.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			template<typename T>
			struct TypeInfoRetriver;

			/// <summary>
			/// A reference holder to an unboxed object.
			/// </summary>
			/// <typeparam name="T">The type of the unboxed object.</typeparam>
			template<typename T>
			struct Unboxed
			{
			private:
				T*				object;
				bool			owned;

			public:
				Unboxed(T* _object, bool _owned) : object(_object), owned(_owned) {}
				Unboxed(Unboxed<T>&& unboxed) : object(unboxed.object), owned(unboxed.owned) { unboxed.object = nullptr; }
				~Unboxed() { if (object && owned) { delete object; } }

				Unboxed() = delete;
				Unboxed(const Unboxed<T>&&) = delete;
				Unboxed<T>& operator=(const Unboxed<T>&) = delete;
				Unboxed<T>& operator=(Unboxed<T>&&) = delete;

				/// <summary>
				/// Get the reference of the unboxed object.
				/// It is recommended only to use this reference when the <see cref="Unboxe`1"/> is still alive.
				/// </summary>
				/// <returns>The unboxed object.</returns>
				T& Ref() const { CHECK_ERROR(object, L"vl::reflection::description::Unboxed<T>::Ref()#The object has been moved away.");  return *object; }

				/// <summary>
				/// Test if the unboxed object is owned.
				/// </summary>
				/// <returns></returns>
				bool IsOwned() const
				{
					return owned;
				}
			};

			template<typename T>
			Value BoxValue(const T& object, ITypeDescriptor* typeDescriptor = nullptr);

			template<typename T>
			T UnboxValue(const Value& value, ITypeDescriptor* typeDescriptor = nullptr, const WString& valueName = WString::Unmanaged(L"value"));

			template<typename T>
			Value BoxParameter(T&& object, ITypeDescriptor* typeDescriptor = nullptr);

			template<typename T>
			Unboxed<T> UnboxParameter(const Value& value, ITypeDescriptor* typeDescriptor = nullptr, const WString& valueName = WString::Unmanaged(L"value"));

#ifndef VCZH_DEBUG_NO_REFLECTION

/***********************************************************************
TypeInfo
***********************************************************************/

#define DECL_TYPE_INFO(TYPENAME) template<>struct TypeInfo<TYPENAME>{ static const TypeInfoContent content; };
#define IMPL_VL_TYPE_INFO(TYPENAME) const TypeInfoContent TypeInfo<TYPENAME>::content = { L ## #TYPENAME, nullptr, TypeInfoContent::VlppType };
#define IMPL_CPP_TYPE_INFO(TYPENAME) const TypeInfoContent TypeInfo<TYPENAME>::content = { L ## #TYPENAME, nullptr, TypeInfoContent::CppType };
#define IMPL_TYPE_INFO_RENAME(TYPENAME, EXPECTEDNAME) const TypeInfoContent TypeInfo<TYPENAME>::content = { L ## #EXPECTEDNAME, L ## #TYPENAME, TypeInfoContent::Renamed };

			struct TypeInfoContent
			{
				enum TypeInfoCppName
				{
					VlppType,			// vl::<type-name>
					CppType,			// <type-name>
					Renamed,			// CppFullTypeName
				};

				const wchar_t*		typeName;
				const wchar_t*		cppFullTypeName;
				TypeInfoCppName		cppName;
			};

			template<typename T>
			struct TypeInfo;

			/// <summary>Get a registered type given a C++ type.</summary>
			/// <returns>Returns the metadata class for this registered type.</returns>
			/// <typeparam name="T">The C++ type to get the registered type.</typeparam>
			/// <remarks>
			/// Returning null means the type registration is declared but the type manager has not started.
			/// Failing to compile means that the type registration is not declared.
			/// See <see cref="Description`1"/> about how to register a type.
			/// </remarks>
			template<typename T>
			ITypeDescriptor* GetTypeDescriptor()
			{
				static vint typeVersion = -1;
				static ITypeDescriptor* cached = nullptr;
				if (auto tm = GetGlobalTypeManager())
				{
					auto currentVersion = tm->GetTypeVersion();
					if (typeVersion != currentVersion)
					{
						cached = GetTypeDescriptor(TypeInfo<T>::content.typeName);
					}
					return cached;
				}
				else
				{
					typeVersion = -1;
					return nullptr;
				}
			}

#endif

#ifndef VCZH_DEBUG_NO_REFLECTION

/***********************************************************************
TypeInfoImp
***********************************************************************/

			class TypeDescriptorTypeInfo : public Object, public ITypeInfo
			{
			protected:
				ITypeDescriptor*						typeDescriptor;
				TypeInfoHint							hint;

			public:
				TypeDescriptorTypeInfo(ITypeDescriptor* _typeDescriptor, TypeInfoHint _hint);
				~TypeDescriptorTypeInfo();

				Decorator								GetDecorator()override;
				TypeInfoHint							GetHint()override;
				ITypeInfo*								GetElementType()override;
				ITypeDescriptor*						GetTypeDescriptor()override;
				vint									GetGenericArgumentCount()override;
				ITypeInfo*								GetGenericArgument(vint index)override;
				WString									GetTypeFriendlyName()override;
			};

			class DecoratedTypeInfo : public Object, public ITypeInfo
			{
			protected:
				Ptr<ITypeInfo>							elementType;

			public:
				DecoratedTypeInfo(Ptr<ITypeInfo> _elementType);
				~DecoratedTypeInfo();

				TypeInfoHint							GetHint()override;
				ITypeInfo*								GetElementType()override;
				ITypeDescriptor*						GetTypeDescriptor()override;
				vint									GetGenericArgumentCount()override;
				ITypeInfo*								GetGenericArgument(vint index)override;
			};

			class RawPtrTypeInfo : public DecoratedTypeInfo
			{
			public:
				RawPtrTypeInfo(Ptr<ITypeInfo> _elementType);
				~RawPtrTypeInfo();

				Decorator								GetDecorator()override;
				WString									GetTypeFriendlyName()override;
			};

			class SharedPtrTypeInfo : public DecoratedTypeInfo
			{
			public:
				SharedPtrTypeInfo(Ptr<ITypeInfo> _elementType);
				~SharedPtrTypeInfo();

				Decorator								GetDecorator()override;
				WString									GetTypeFriendlyName()override;
			};

			class NullableTypeInfo : public DecoratedTypeInfo
			{
			public:
				NullableTypeInfo(Ptr<ITypeInfo> _elementType);
				~NullableTypeInfo();

				Decorator								GetDecorator()override;
				WString									GetTypeFriendlyName()override;
			};

			class GenericTypeInfo : public DecoratedTypeInfo
			{
			protected:
				collections::List<Ptr<ITypeInfo>>		genericArguments;

			public:
				GenericTypeInfo(Ptr<ITypeInfo> _elementType);
				~GenericTypeInfo();

				Decorator								GetDecorator()override;
				vint									GetGenericArgumentCount()override;
				ITypeInfo*								GetGenericArgument(vint index)override;
				WString									GetTypeFriendlyName()override;

				void									AddGenericArgument(Ptr<ITypeInfo> value);
			};

#endif

#ifndef VCZH_DEBUG_NO_REFLECTION

/***********************************************************************
SerializableTypeDescriptor
***********************************************************************/

			class TypeDescriptorImplBase : public Object, public ITypeDescriptor, private ITypeDescriptor::ICpp
			{
			private:
				TypeDescriptorFlags							typeDescriptorFlags;
				const TypeInfoContent*						typeInfoContent;
				WString										typeName;
				WString										cppFullTypeName;

				const WString&								GetFullName()override;

			protected:
				const TypeInfoContent*						GetTypeInfoContentInternal();

			public:
				TypeDescriptorImplBase(TypeDescriptorFlags _typeDescriptorFlags, const TypeInfoContent* _typeInfoContent);
				~TypeDescriptorImplBase();

				ITypeDescriptor::ICpp*						GetCpp()override;
				TypeDescriptorFlags							GetTypeDescriptorFlags()override;
				const WString&								GetTypeName()override;
			};

			class ValueTypeDescriptorBase : public TypeDescriptorImplBase
			{
			protected:
				bool										loaded;
				Ptr<IValueType>								valueType;
				Ptr<IEnumType>								enumType;
				Ptr<ISerializableType>						serializableType;

				virtual void								LoadInternal();;
				void										Load();
			public:
				ValueTypeDescriptorBase(TypeDescriptorFlags _typeDescriptorFlags, const TypeInfoContent* _typeInfoContent);
				~ValueTypeDescriptorBase();

				bool										IsAggregatable()override;
				IValueType*									GetValueType()override;
				IEnumType*									GetEnumType()override;
				ISerializableType*							GetSerializableType()override;

				vint										GetBaseTypeDescriptorCount()override;
				ITypeDescriptor*							GetBaseTypeDescriptor(vint index)override;
				bool										CanConvertTo(ITypeDescriptor* targetType)override;
				vint										GetPropertyCount()override;
				IPropertyInfo*								GetProperty(vint index)override;
				bool										IsPropertyExists(const WString& name, bool inheritable)override;
				IPropertyInfo*								GetPropertyByName(const WString& name, bool inheritable)override;
				vint										GetEventCount()override;
				IEventInfo*									GetEvent(vint index)override;
				bool										IsEventExists(const WString& name, bool inheritable)override;
				IEventInfo*									GetEventByName(const WString& name, bool inheritable)override;
				vint										GetMethodGroupCount()override;
				IMethodGroupInfo*							GetMethodGroup(vint index)override;
				bool										IsMethodGroupExists(const WString& name, bool inheritable)override;
				IMethodGroupInfo*							GetMethodGroupByName(const WString& name, bool inheritable)override;
				IMethodGroupInfo*							GetConstructorGroup()override;
			};

			template<typename T, TypeDescriptorFlags TDFlags>
			class TypedValueTypeDescriptorBase : public ValueTypeDescriptorBase
			{
			public:
				TypedValueTypeDescriptorBase()
					:ValueTypeDescriptorBase(TDFlags, &TypeInfo<T>::content)
				{
				}
			};

/***********************************************************************
ParameterInfoImpl
***********************************************************************/

			class ParameterInfoImpl : public Object, public IParameterInfo
			{
			protected:
				IMethodInfo*							ownerMethod;
				WString									name;
				Ptr<ITypeInfo>							type;
			public:
				ParameterInfoImpl(IMethodInfo* _ownerMethod, const WString& _name, Ptr<ITypeInfo> _type);
				~ParameterInfoImpl();

				ITypeDescriptor*						GetOwnerTypeDescriptor()override;
				const WString&							GetName()override;
				ITypeInfo*								GetType()override;
				IMethodInfo*							GetOwnerMethod()override;
			};

/***********************************************************************
MethodInfoImpl
***********************************************************************/

			class MethodInfoImpl : public Object, public IMethodInfo
			{
				friend class PropertyInfoImpl;
			protected:
				IMethodGroupInfo*						ownerMethodGroup;
				IPropertyInfo*							ownerProperty;
				collections::List<Ptr<IParameterInfo>>	parameters;
				Ptr<ITypeInfo>							returnInfo;
				bool									isStatic;

				virtual Value							InvokeInternal(const Value& thisObject, collections::Array<Value>& arguments)=0;
				virtual Value							CreateFunctionProxyInternal(const Value& thisObject) = 0;
			public:
				MethodInfoImpl(IMethodGroupInfo* _ownerMethodGroup, Ptr<ITypeInfo> _return, bool _isStatic);
				~MethodInfoImpl();

				ITypeDescriptor*						GetOwnerTypeDescriptor()override;
				IPropertyInfo*							GetOwnerProperty()override;
				const WString&							GetName()override;
				IMethodGroupInfo*						GetOwnerMethodGroup()override;
				vint									GetParameterCount()override;
				IParameterInfo*							GetParameter(vint index)override;
				ITypeInfo*								GetReturn()override;
				bool									IsStatic()override;
				void									CheckArguments(collections::Array<Value>& arguments)override;
				Value									Invoke(const Value& thisObject, collections::Array<Value>& arguments)override;
				Value									CreateFunctionProxy(const Value& thisObject)override;
				bool									AddParameter(Ptr<IParameterInfo> parameter);
				bool									SetOwnerMethodgroup(IMethodGroupInfo* _ownerMethodGroup);
			};

/***********************************************************************
MethodGroupInfoImpl
***********************************************************************/

			class MethodGroupInfoImpl : public Object, public IMethodGroupInfo
			{
			protected:
				ITypeDescriptor*						ownerTypeDescriptor;
				WString									name;
				collections::List<Ptr<IMethodInfo>>		methods;
			public:
				MethodGroupInfoImpl(ITypeDescriptor* _ownerTypeDescriptor, const WString& _name);
				~MethodGroupInfoImpl();

				ITypeDescriptor*						GetOwnerTypeDescriptor()override;
				const WString&							GetName()override;
				vint									GetMethodCount()override;
				IMethodInfo*							GetMethod(vint index)override;
				bool									AddMethod(Ptr<IMethodInfo> _method);
			};

/***********************************************************************
EventInfoImpl
***********************************************************************/

			class EventInfoImpl : public Object, public IEventInfo
			{
				friend class PropertyInfoImpl;

			protected:
				ITypeDescriptor*						ownerTypeDescriptor;
				collections::List<IPropertyInfo*>		observingProperties;
				WString									name;
				Ptr<ITypeInfo>							handlerType;

				virtual Ptr<IEventHandler>				AttachInternal(DescriptableObject* thisObject, Ptr<IValueFunctionProxy> handler)=0;
				virtual bool							DetachInternal(DescriptableObject* thisObject, Ptr<IEventHandler> handler)=0;
				virtual void							InvokeInternal(DescriptableObject* thisObject, Ptr<IValueReadonlyList> arguments)=0;
				virtual Ptr<ITypeInfo>					GetHandlerTypeInternal()=0;
			public:
				EventInfoImpl(ITypeDescriptor* _ownerTypeDescriptor, const WString& _name);
				~EventInfoImpl();

				ITypeDescriptor*						GetOwnerTypeDescriptor()override;
				const WString&							GetName()override;
				ITypeInfo*								GetHandlerType()override;
				vint									GetObservingPropertyCount()override;
				IPropertyInfo*							GetObservingProperty(vint index)override;
				Ptr<IEventHandler>						Attach(const Value& thisObject, Ptr<IValueFunctionProxy> handler)override;
				bool									Detach(const Value& thisObject, Ptr<IEventHandler> handler)override;
				void									Invoke(const Value& thisObject, Ptr<IValueReadonlyList> arguments)override;
			};

/***********************************************************************
TypeDescriptorImpl
***********************************************************************/

			class PropertyInfoImpl : public Object, public IPropertyInfo
			{
			protected:
				ITypeDescriptor*						ownerTypeDescriptor;
				WString									name;
				Ptr<ICpp>								cpp;
				MethodInfoImpl*							getter;
				MethodInfoImpl*							setter;
				EventInfoImpl*							valueChangedEvent;

			public:
				PropertyInfoImpl(ITypeDescriptor* _ownerTypeDescriptor, const WString& _name, MethodInfoImpl* _getter, MethodInfoImpl* _setter, EventInfoImpl* _valueChangedEvent);
				~PropertyInfoImpl();

				ITypeDescriptor*						GetOwnerTypeDescriptor()override;
				const WString&							GetName()override;
				IPropertyInfo::ICpp*					GetCpp()override;

				bool									IsReadable()override;
				bool									IsWritable()override;
				ITypeInfo*								GetReturn()override;
				IMethodInfo*							GetGetter()override;
				IMethodInfo*							GetSetter()override;
				IEventInfo*								GetValueChangedEvent()override;
				Value									GetValue(const Value& thisObject)override;
				void									SetValue(Value& thisObject, const Value& newValue)override;
			};

			class PropertyInfoImpl_StaticCpp : public PropertyInfoImpl, private IPropertyInfo::ICpp
			{
			private:
				WString									referenceTemplate;

				const WString&							GetReferenceTemplate()override;

			public:
				PropertyInfoImpl_StaticCpp(ITypeDescriptor* _ownerTypeDescriptor, const WString& _name, MethodInfoImpl* _getter, MethodInfoImpl* _setter, EventInfoImpl* _valueChangedEvent, const WString& _referenceTemplate);
				~PropertyInfoImpl_StaticCpp();

				IPropertyInfo::ICpp*					GetCpp()override;
			};

/***********************************************************************
FieldInfoImpl
***********************************************************************/

			class FieldInfoImpl : public Object, public IPropertyInfo
			{
			protected:
				ITypeDescriptor*						ownerTypeDescriptor;
				Ptr<ITypeInfo>							returnInfo;
				WString									name;

				virtual Value							GetValueInternal(const Value& thisObject)=0;
				virtual void							SetValueInternal(Value& thisObject, const Value& newValue)=0;
			public:
				FieldInfoImpl(ITypeDescriptor* _ownerTypeDescriptor, const WString& _name, Ptr<ITypeInfo> _returnInfo);
				~FieldInfoImpl();

				ITypeDescriptor*						GetOwnerTypeDescriptor()override;
				const WString&							GetName()override;
				bool									IsReadable()override;
				bool									IsWritable()override;
				ITypeInfo*								GetReturn()override;
				IMethodInfo*							GetGetter()override;
				IMethodInfo*							GetSetter()override;
				IEventInfo*								GetValueChangedEvent()override;
				Value									GetValue(const Value& thisObject)override;
				void									SetValue(Value& thisObject, const Value& newValue)override;
			};

/***********************************************************************
TypeDescriptorImpl
***********************************************************************/

			class TypeDescriptorImpl : public TypeDescriptorImplBase
			{
			private:
				bool														loaded;
				collections::List<ITypeDescriptor*>							baseTypeDescriptors;
				collections::Dictionary<WString, Ptr<IPropertyInfo>>		properties;
				collections::Dictionary<WString, Ptr<IEventInfo>>			events;
				collections::Dictionary<WString, Ptr<MethodGroupInfoImpl>>	methodGroups;
				Ptr<MethodGroupInfoImpl>									constructorGroup;

			protected:
				MethodGroupInfoImpl*		PrepareMethodGroup(const WString& name);
				MethodGroupInfoImpl*		PrepareConstructorGroup();
				IPropertyInfo*				AddProperty(Ptr<IPropertyInfo> value);
				IEventInfo*					AddEvent(Ptr<IEventInfo> value);
				IMethodInfo*				AddMethod(const WString& name, Ptr<MethodInfoImpl> value);
				IMethodInfo*				AddConstructor(Ptr<MethodInfoImpl> value);
				void						AddBaseType(ITypeDescriptor* value);

				virtual void				LoadInternal()=0;
				void						Load();
			public:
				TypeDescriptorImpl(TypeDescriptorFlags _typeDescriptorFlags, const TypeInfoContent* _typeInfoContent);
				~TypeDescriptorImpl();

				bool						IsAggregatable()override;
				IValueType*					GetValueType()override;
				IEnumType*					GetEnumType()override;
				ISerializableType*			GetSerializableType()override;

				vint						GetBaseTypeDescriptorCount()override;
				ITypeDescriptor*			GetBaseTypeDescriptor(vint index)override;
				bool						CanConvertTo(ITypeDescriptor* targetType)override;

				vint						GetPropertyCount()override;
				IPropertyInfo*				GetProperty(vint index)override;
				bool						IsPropertyExists(const WString& name, bool inheritable)override;
				IPropertyInfo*				GetPropertyByName(const WString& name, bool inheritable)override;

				vint						GetEventCount()override;
				IEventInfo*					GetEvent(vint index)override;
				bool						IsEventExists(const WString& name, bool inheritable)override;
				IEventInfo*					GetEventByName(const WString& name, bool inheritable)override;

				vint						GetMethodGroupCount()override;
				IMethodGroupInfo*			GetMethodGroup(vint index)override;
				bool						IsMethodGroupExists(const WString& name, bool inheritable)override;
				IMethodGroupInfo*			GetMethodGroupByName(const WString& name, bool inheritable)override;
				IMethodGroupInfo*			GetConstructorGroup()override;
			};

#endif

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

/***********************************************************************
CustomFieldInfoImpl
***********************************************************************/

			template<typename TClass, typename TField>
			class CustomFieldInfoImpl : public FieldInfoImpl
			{
			protected:
				TField TClass::*				fieldRef;

				Value GetValueInternal(const Value& thisObject)override
				{
					TClass* object = UnboxValue<TClass*>(thisObject);
					if (object)
					{
						return BoxParameter(object->*fieldRef, GetReturn()->GetTypeDescriptor());
					}
					return Value();
				}

				void SetValueInternal(Value& thisObject, const Value& newValue)override
				{
					if constexpr (std::is_copy_assignable_v<TField>)
					{
						TClass* object = UnboxValue<TClass*>(thisObject);
						if (object)
						{
							auto result = UnboxParameter<TField>(newValue, GetReturn()->GetTypeDescriptor(), L"newValue");
							object->*fieldRef = result.Ref();
						}
					}
					else
					{
						throw PropertyIsNotWritableException(this);
					}
				}
			public:
				CustomFieldInfoImpl(ITypeDescriptor* _ownerTypeDescriptor, const WString& _name, TField TClass::* _fieldRef)
					:FieldInfoImpl(_ownerTypeDescriptor, _name, TypeInfoRetriver<TField>::CreateTypeInfo())
					, fieldRef(_fieldRef)
				{
				}

				IPropertyInfo::ICpp* GetCpp()override
				{
					return nullptr;
				}

				bool IsWritable()override
				{
					return std::is_copy_assignable_v<TField>;
				}
			};

#endif

/***********************************************************************
PrimitiveTypeDescriptor
***********************************************************************/

#ifndef VCZH_DEBUG_NO_REFLECTION

			template<typename T>
			class SerializableValueType : public Object, public virtual IValueType
			{
			public:
				Value CreateDefault()override
				{
					return BoxValue<T>(TypedValueSerializerProvider<T>::GetDefaultValue());
				}

				IBoxedValue::CompareResult Compare(const Value& a, const Value& b)override
				{
					auto va = UnboxValue<T>(a);
					auto vb = UnboxValue<T>(b);
					return TypedValueSerializerProvider<T>::Compare(va, vb);
				}
			};

			template<typename T>
			class SerializableType : public Object, public virtual ISerializableType
			{
			public:
				bool Serialize(const Value& input, WString& output)override
				{
					return TypedValueSerializerProvider<T>::Serialize(UnboxValue<T>(input), output);
				}

				bool Deserialize(const WString& input, Value& output)override
				{
					T value;
					if (!TypedValueSerializerProvider<T>::Deserialize(input, value))
					{
						return false;
					}
					output = BoxValue<T>(value);
					return true;
				}
			};

#endif

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

			template<typename T>
			class PrimitiveTypeDescriptor : public TypedValueTypeDescriptorBase<T, TypeDescriptorFlags::Primitive>
			{
			protected:
				void LoadInternal()override
				{
					this->valueType = new SerializableValueType<T>();
					this->serializableType = new SerializableType<T>();
				}
			};

/***********************************************************************
EnumTypeDescriptor
***********************************************************************/

			template<typename T>
			class EnumValueType : public Object, public virtual IValueType
			{
			public:
				Value CreateDefault()override
				{
					return BoxValue<T>(static_cast<T>(0));
				}

				IBoxedValue::CompareResult Compare(const Value& a, const Value& b)override
				{
					auto ea = static_cast<vuint64_t>(UnboxValue<T>(a));
					auto eb = static_cast<vuint64_t>(UnboxValue<T>(b));
					if (ea < eb) return IBoxedValue::Smaller;
					if (ea > eb)return IBoxedValue::Greater;
					return IBoxedValue::Equal;
				}
			};

			template<typename T, bool Flag>
			class EnumType : public Object, public virtual IEnumType
			{
			protected:
				collections::Dictionary<WString, T>			candidates;

			public:
				void AddItem(WString name, T value)
				{
					candidates.Add(name, value);
				}

				bool IsFlagEnum()override
				{
					return Flag;
				}

				vint GetItemCount()override
				{
					return candidates.Count();
				}

				WString GetItemName(vint index)override
				{
					if (index < 0 || index >= candidates.Count())
					{
						return L"";
					}
					return candidates.Keys()[index];
				}

				vuint64_t GetItemValue(vint index)override
				{
					if (index < 0 || index >= candidates.Count())
					{
						return 0;
					}
					return static_cast<vuint64_t>(candidates.Values()[index]);
				}

				vint IndexOfItem(WString name)override
				{
					return candidates.Keys().IndexOf(name);
				}

				Value ToEnum(vuint64_t value)override
				{
					return BoxValue<T>(static_cast<T>(value));
				}

				vuint64_t FromEnum(const Value& value)override
				{
					return static_cast<vuint64_t>(UnboxValue<T>(value));
				}
			};

			template<typename T, TypeDescriptorFlags TDFlags>
			class EnumTypeDescriptor : public TypedValueTypeDescriptorBase<T, TDFlags>
			{
				using TEnumType = EnumType<T, TDFlags == TypeDescriptorFlags::FlagEnum>;
			protected:
				Ptr<TEnumType>					enumType;

				void LoadInternal()override
				{
					this->enumType = new TEnumType;
					this->valueType = new EnumValueType<T>();
					TypedValueTypeDescriptorBase<T, TDFlags>::enumType = enumType;
				}
			};

/***********************************************************************
StructTypeDescriptor
***********************************************************************/

			template<typename T>
			class StructValueType : public Object, public virtual IValueType
			{
			public:
				Value CreateDefault()override
				{
					return BoxValue<T>(T{});
				}

				IBoxedValue::CompareResult Compare(const Value& a, const Value& b)override
				{
					return IBoxedValue::NotComparable;
				}
			};

			template<typename T, TypeDescriptorFlags TDFlags>
			class StructTypeDescriptor : public TypedValueTypeDescriptorBase<T, TDFlags>
			{
			protected:
				template<typename TField>
				class StructFieldInfo : public FieldInfoImpl
				{
				protected:
					TField T::*					field;

					Value GetValueInternal(const Value& thisObject)override
					{
						auto structValue = thisObject.GetBoxedValue().Cast<IValueType::TypedBox<T>>();
						if (!structValue)
						{
							throw ArgumentTypeMismtatchException(L"thisObject", GetOwnerTypeDescriptor(), Value::BoxedValue, thisObject);
						}
						return BoxValue<TField>(structValue->value.*field);
					}

					void SetValueInternal(Value& thisObject, const Value& newValue)override
					{
						auto structValue = thisObject.GetBoxedValue().Cast<IValueType::TypedBox<T>>();
						if (!structValue)
						{
							throw ArgumentTypeMismtatchException(L"thisObject", GetOwnerTypeDescriptor(), Value::BoxedValue, thisObject);
						}
						(structValue->value.*field) = UnboxValue<TField>(newValue);
					}
				public:
					StructFieldInfo(ITypeDescriptor* _ownerTypeDescriptor, TField T::* _field, const WString& _name)
						:field(_field)
						, FieldInfoImpl(_ownerTypeDescriptor, _name, TypeInfoRetriver<TField>::CreateTypeInfo())
					{
					}

					IPropertyInfo::ICpp* GetCpp()override
					{
						return nullptr;
					}
				};

			protected:
				collections::Dictionary<WString, Ptr<IPropertyInfo>>		fields;

			public:
				StructTypeDescriptor()
				{
					this->valueType = new StructValueType<T>();
				}

				vint GetPropertyCount()override
				{
					this->Load();
					return fields.Count();
				}

				IPropertyInfo* GetProperty(vint index)override
				{
					this->Load();
					if (index < 0 || index >= fields.Count())
					{
						return nullptr;
					}
					return fields.Values()[index].Obj();
				}

				bool IsPropertyExists(const WString& name, bool inheritable)override
				{
					this->Load();
					return fields.Keys().Contains(name);
				}

				IPropertyInfo* GetPropertyByName(const WString& name, bool inheritable)override
				{
					this->Load();
					vint index = fields.Keys().IndexOf(name);
					if (index == -1) return nullptr;
					return fields.Values()[index].Obj();
				}
			};
#endif
		}
	}
}

#endif
