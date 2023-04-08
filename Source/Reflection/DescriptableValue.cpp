/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "DescriptableInterfaces.h"
#include "./Reflection/Reflection.h"

namespace vl
{
	using namespace collections;

	namespace reflection
	{

/***********************************************************************
description::Value
***********************************************************************/

		namespace description
		{
			Value::Value(DescriptableObject* value)
				:valueType(value ? RawPtr :Null)
				,rawPtr(nullptr)
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				,typeDescriptor(nullptr)
#endif
			{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				if (value)
				{
					rawPtr = value->SafeGetAggregationRoot();
				}
#else
				rawPtr = value;
#endif
			}

			Value::Value(Ptr<DescriptableObject> value)
				:valueType(value ? SharedPtr : Null)
				,rawPtr(nullptr)
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				,typeDescriptor(nullptr)
#endif
			{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				if (value)
				{
					rawPtr = value->SafeGetAggregationRoot();
					sharedPtr = Ptr(rawPtr);
				}
#else
				rawPtr = value.Obj();
				sharedPtr = value;
#endif
			}

			Value::Value(Ptr<IBoxedValue> value, ITypeDescriptor* associatedTypeDescriptor)
				:valueType(value ? BoxedValue : Null)
				, rawPtr(nullptr)
				, boxedValue(value)
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				, typeDescriptor(associatedTypeDescriptor)
#endif
			{
			}

			Value::Value()
				:valueType(Null)
				,rawPtr(0)
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				,typeDescriptor(nullptr)
#endif
			{
			}

			Value::Value(const Value& value)
				:valueType(value.valueType)
				,rawPtr(value.rawPtr)
				,sharedPtr(value.sharedPtr)
				,boxedValue(value.boxedValue ? value.boxedValue->Copy() : nullptr)
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				,typeDescriptor(value.typeDescriptor)
#endif
			{
			}

			Value& Value::operator=(const Value& value)
			{
				valueType = value.valueType;
				rawPtr = value.rawPtr;
				sharedPtr = value.sharedPtr;
				boxedValue = value.boxedValue ? value.boxedValue->Copy() : nullptr;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				typeDescriptor = value.typeDescriptor;
#endif
				return *this;
			}

			std::strong_ordering operator<=>(const Value& a, const Value& b)
			{
				switch (a.GetValueType())
				{
				case Value::RawPtr:
				case Value::SharedPtr:
					switch (b.GetValueType())
					{
					case Value::RawPtr:
					case Value::SharedPtr:
						{
							auto pa = a.GetRawPtr();
							auto pb = b.GetRawPtr();
							return pa <=> pb;
						}
					case Value::BoxedValue:
						return std::strong_ordering::less;
					default:
						return std::strong_ordering::greater;
					}
				case Value::BoxedValue:
					switch (b.GetValueType())
					{
					case Value::RawPtr:
					case Value::SharedPtr:
						return std::strong_ordering::greater;
					case Value::BoxedValue:
						{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
							auto aSt = a.GetTypeDescriptor()->GetSerializableType();
							auto bSt = b.GetTypeDescriptor()->GetSerializableType();
							if (aSt)
							{
								if (bSt)
								{
									auto aSt = a.GetTypeDescriptor()->GetSerializableType();
									auto bSt = b.GetTypeDescriptor()->GetSerializableType();

									WString aText;
									WString bText;
									aSt->Serialize(a, aText);
									bSt->Serialize(b, bText);
									return aText <=> bText;
								}
								else
								{
									return std::strong_ordering::greater;
								}
							}
							else
							{
								if (bSt)
								{
									return std::strong_ordering::less;
								}
								else
								{
									if (a.GetTypeDescriptor() != b.GetTypeDescriptor())
									{
										auto aText = a.GetTypeDescriptor()->GetTypeName();
										auto bText = b.GetTypeDescriptor()->GetTypeName();
										return aText <=> bText;
									}

									switch (a.GetTypeDescriptor()->GetTypeDescriptorFlags())
									{
									case TypeDescriptorFlags::Struct:
										{
											auto td = a.GetTypeDescriptor();
											vint count = td->GetPropertyCount();
											for (vint i = 0; i < count; i++)
											{
												auto prop = td->GetProperty(i);
												auto ap = prop->GetValue(a);
												auto bp = prop->GetValue(b);
												return ap <=> bp;
											}
										}
										return std::strong_ordering::equal;
									case TypeDescriptorFlags::FlagEnum:
									case TypeDescriptorFlags::NormalEnum:
										{
											auto ai = a.GetTypeDescriptor()->GetEnumType()->FromEnum(a);
											auto bi = a.GetTypeDescriptor()->GetEnumType()->FromEnum(b);
											return ai <=> bi;
										}
									default:
										return std::strong_ordering::equal;
									}
								}
							}
#else
								auto pa = a.GetBoxedValue();
								auto pb = b.GetBoxedValue();
								switch (pa->ComparePrimitive(pb))
								{
								case IBoxedValue::Smaller: return std::strong_ordering::less;
								case IBoxedValue::Greater: return std::strong_ordering::greater;
								case IBoxedValue::Equal: return std::strong_ordering::equal;
								default:;
								}
								return pa.Obj() <=> pb.Obj();
#endif
						}
					default:
						return std::strong_ordering::greater;
					}
				default:
					switch (b.GetValueType())
					{
					case Value::RawPtr:
					case Value::SharedPtr:
					case Value::BoxedValue:
						return std::strong_ordering::less;
					default:
						return std::strong_ordering::equal;
					}
				}
			}

			Value::ValueType Value::GetValueType()const
			{
				return valueType;
			}

			DescriptableObject* Value::GetRawPtr()const
			{
				return rawPtr;
			}

			Ptr<DescriptableObject> Value::GetSharedPtr()const
			{
				return sharedPtr;
			}

			Ptr<IBoxedValue> Value::GetBoxedValue()const
			{
				return boxedValue;
			}

			bool Value::IsNull()const
			{
				return valueType == Null;
			}

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

			ITypeDescriptor* Value::GetTypeDescriptor()const
			{
				switch(valueType)
				{
				case RawPtr:
				case SharedPtr:
					return rawPtr?rawPtr->GetTypeDescriptor():0;
				case BoxedValue:
					return typeDescriptor;
				default:;
				}
				return 0;
			}

			WString Value::GetTypeFriendlyName()const
			{
				switch(valueType)
				{
				case RawPtr:
					return GetTypeDescriptor()->GetTypeName()+L"*";
				case SharedPtr:
					return L"Ptr<"+GetTypeDescriptor()->GetTypeName()+L">";
				case BoxedValue:
					return GetTypeDescriptor()->GetTypeName();
				default:
					return L"null";
				}
			}

			bool Value::CanConvertTo(ITypeDescriptor* targetType, ValueType targetValueType)const
			{
				if(targetType==GetGlobalTypeManager()->GetRootType())
				{
					return true;
				}
				switch(valueType)
				{
				case Null:
					return targetValueType != BoxedValue;
				case RawPtr:
				case SharedPtr:
					if (targetValueType != RawPtr && targetValueType != SharedPtr) return false;
					break;
				case BoxedValue:
					return targetValueType == BoxedValue;
				}
				return GetTypeDescriptor()->CanConvertTo(targetType);
			}

			bool Value::CanConvertTo(ITypeInfo* targetType)const
			{
				if(valueType==Null && targetType->GetDecorator()==ITypeInfo::Nullable)
				{
					return true;
				}
				ValueType targetValueType=ValueType::Null;
				{
					ITypeInfo* currentType=targetType;
					while(currentType)
					{
						switch(targetType->GetDecorator())
						{
						case ITypeInfo::RawPtr:
							targetValueType=RawPtr;
							currentType=0;
							break;
						case ITypeInfo::SharedPtr:
							targetValueType=SharedPtr;
							currentType=0;
							break;
						case ITypeInfo::TypeDescriptor:
						case ITypeInfo::Nullable:
							targetValueType=BoxedValue;
							currentType=0;
							break;
						default:
							currentType=currentType->GetElementType();
						}
					}
				}
				return CanConvertTo(targetType->GetTypeDescriptor(), targetValueType);
			}

#endif

			Value Value::From(DescriptableObject* value)
			{
				return Value(value);
			}

			Value Value::From(Ptr<DescriptableObject> value)
			{
				return Value(value);
			}

			Value Value::From(Ptr<IBoxedValue> value, ITypeDescriptor* type)
			{
				return Value(value, type);
			}

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

			IMethodInfo* Value::SelectMethod(IMethodGroupInfo* methodGroup, collections::Array<Value>& arguments)
			{
				if(methodGroup->GetMethodCount()==1)
				{
					return methodGroup->GetMethod(0);
				}

				List<IMethodInfo*> methods;
				for(vint i=0;i<methodGroup->GetMethodCount();i++)
				{
					IMethodInfo* method=methodGroup->GetMethod(i);
					if(method->GetParameterCount()==arguments.Count())
					{
						methods.Add(method);
					}
				}

				if(methods.Count()==0)
				{
					throw ArgumentCountMismtatchException(methodGroup);
				}
				else if(methods.Count()==1)
				{
					return methods[0];
				}
				else
				{
					for(vint i=0;i<methods.Count();i++)
					{
						IMethodInfo* method=methods[i];
						try
						{
							method->CheckArguments(arguments);
							return method;
						}
						catch(const TypeDescriptorException&)
						{
						}
					}
					return methods[0];
				}
			}

			Value Value::Create(ITypeDescriptor* type)
			{
				Array<Value> arguments;
				return Create(type, arguments);
			}

			Value Value::Create(ITypeDescriptor* type, collections::Array<Value>& arguments)
			{
				IMethodGroupInfo* methodGroup=type->GetConstructorGroup();
				if(!methodGroup) throw ConstructorNotExistsException(type);

				IMethodInfo* method=SelectMethod(methodGroup, arguments);
				return method->Invoke(Value(), arguments);
			}

			Value Value::Create(const WString& typeName)
			{
				Array<Value> arguments;
				return Create(typeName, arguments);
			}

			Value Value::Create(const WString& typeName, collections::Array<Value>& arguments)
			{
				ITypeDescriptor* type = vl::reflection::description::GetTypeDescriptor(typeName);
				if(!type) throw TypeNotExistsException(typeName);
				return Create(type, arguments);
			}

			Value Value::InvokeStatic(const WString& typeName, const WString& name)
			{
				Array<Value> arguments;
				return InvokeStatic(typeName, name, arguments);
			}

			Value Value::InvokeStatic(const WString& typeName, const WString& name, collections::Array<Value>& arguments)
			{
				ITypeDescriptor* type=vl::reflection::description::GetTypeDescriptor(typeName);
				if(!type) throw TypeNotExistsException(typeName);

				IMethodGroupInfo* methodGroup=type->GetMethodGroupByName(name, true);
				if(!methodGroup) throw MemberNotExistsException(name, type);

				IMethodInfo* method=SelectMethod(methodGroup, arguments);
				return method->Invoke(Value(), arguments);
			}

			Value Value::GetProperty(const WString& name)const
			{
				ITypeDescriptor* type=GetTypeDescriptor();
				if(!type) throw ArgumentNullException(L"thisObject", name);

				IPropertyInfo* prop=type->GetPropertyByName(name, true);
				if(!prop) throw MemberNotExistsException(name, type);

				return prop->GetValue(*this);
			}

			void Value::SetProperty(const WString& name, const Value& newValue)
			{
				ITypeDescriptor* type=GetTypeDescriptor();
				if(!type) throw ArgumentNullException(L"thisObject", name);

				IPropertyInfo* prop=type->GetPropertyByName(name, true);
				if(!prop) throw MemberNotExistsException(name, type);

				prop->SetValue(*this, newValue);
			}

			Value Value::Invoke(const WString& name)const
			{
				Array<Value> arguments;
				return Invoke(name, arguments);
			}

			Value Value::Invoke(const WString& name, collections::Array<Value>& arguments)const
			{
				ITypeDescriptor* type=GetTypeDescriptor();
				if(!type) throw ArgumentNullException(L"thisObject", name);

				IMethodGroupInfo* methodGroup=type->GetMethodGroupByName(name, true);
				if(!methodGroup) throw MemberNotExistsException(name, type);

				IMethodInfo* method=SelectMethod(methodGroup, arguments);
				return method->Invoke(*this, arguments);
			}

			Ptr<IEventHandler> Value::AttachEvent(const WString& name, const Value& function)const
			{
				ITypeDescriptor* type=GetTypeDescriptor();
				if(!type) throw ArgumentNullException(L"thisObject", name);

				IEventInfo* eventInfo=type->GetEventByName(name, true);
				if(!eventInfo) throw MemberNotExistsException(name, type);

				Ptr<IValueFunctionProxy> proxy=UnboxValue<Ptr<IValueFunctionProxy>>(function, description::GetTypeDescriptor<IValueFunctionProxy>(), L"function");
				return eventInfo->Attach(*this, proxy);
			}

			bool Value::DetachEvent(const WString& name, Ptr<IEventHandler> handler)const
			{
				ITypeDescriptor* type = GetTypeDescriptor();
				if (!type) throw ArgumentNullException(L"thisObject", name);

				IEventInfo* eventInfo = type->GetEventByName(name, true);
				if (!eventInfo) throw MemberNotExistsException(name, type);

				return eventInfo->Detach(*this, handler);
			}

#endif

			bool Value::DeleteRawPtr()
			{
				if(valueType!=RawPtr) return false;
				if(!rawPtr) return false;
				rawPtr->Dispose(true);
				*this=Value();
				return true;
			}
		}
	}
}
