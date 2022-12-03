/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_BOXING_BOXINGVALUE
#define VCZH_REFLECTION_BOXING_BOXINGVALUE

#include "TypeInfoRetriver.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
/***********************************************************************
BoxValue, UnboxValue
***********************************************************************/

			template<typename T, ITypeInfo::Decorator Decorator>
			struct ValueAccessor
			{
			};

			/// <summary>Box an reflectable object. Its type cannot be generic.</summary>
			/// <returns>The boxed value.</returns>
			/// <typeparam name="T">Type of the object.</typeparam>
			/// <param name="object">The object to box.</param>
			/// <param name="typeDescriptor">The type descriptor of the object (optional).</param>
			template<typename T>
			Value BoxValue(const T& object, ITypeDescriptor* typeDescriptor)
			{
				using Type = std::remove_cvref_t<T>;
				return ValueAccessor<Type, TypeInfoRetriver<Type>::Decorator>::BoxValue(object, typeDescriptor);
			}
			
			/// <summary>Unbox a reflectable object. Its type cannot be generic.</summary>
			/// <returns>The unboxed object.</returns>
			/// <typeparam name="T">Type of the object.</typeparam>
			/// <param name="value">The value to unbox.</param>
			/// <param name="typeDescriptor">The type descriptor of the object (optional).</param>
			/// <param name="valueName">The name of the object to provide a friendly exception message if the conversion is failed (optional).</param>
			template<typename T>
			T UnboxValue(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
			{
				using Type = std::remove_cvref_t<T>;
				return ValueAccessor<Type, TypeInfoRetriver<Type>::Decorator>::UnboxValue(value, typeDescriptor, valueName);
			}

/***********************************************************************
Basic Types
***********************************************************************/

			template<typename T>
			struct ValueAccessor<T*, ITypeInfo::RawPtr>
			{
				static Value BoxValue(T* object, ITypeDescriptor* typeDescriptor)
				{
					return Value::From(object);
				}

				static T* UnboxValue(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					if(value.IsNull()) return nullptr;
					T* result = nullptr;
					if (value.GetRawPtr())
					{
						result = value.GetRawPtr()->SafeAggregationCast<T>();
					}
					if(!result)
					{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
						if(!typeDescriptor)
						{
							typeDescriptor=GetTypeDescriptor<T>();
						}
						throw ArgumentTypeMismtatchException(valueName, typeDescriptor, Value::RawPtr, value);
#else
						CHECK_FAIL(L"vl::reflection::description::UnboxValue()#Argument type mismatch.");
#endif
					}
					return result;
				}
			};

			template<typename T>
			struct ValueAccessor<Ptr<T>, ITypeInfo::SharedPtr>
			{
				static Value BoxValue(Ptr<T> object, ITypeDescriptor* typeDescriptor)
				{
					return Value::From(object);
				}

				static Ptr<T> UnboxValue(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					if (value.IsNull()) return nullptr;
					Ptr<T> result;
					if(value.GetValueType()==Value::RawPtr || value.GetValueType()==Value::SharedPtr)
					{
						result = Ptr(value.GetRawPtr()->SafeAggregationCast<T>());
					}
					if(!result)
					{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
						if(!typeDescriptor)
						{
							typeDescriptor=GetTypeDescriptor<T>();
						}
						throw ArgumentTypeMismtatchException(valueName, typeDescriptor, Value::SharedPtr, value);
#else
						CHECK_FAIL(L"vl::reflection::description::UnboxValue()#Argument type mismatch.");
#endif
					}
					return result;
				}
			};

			template<typename T>
			struct ValueAccessor<Nullable<T>, ITypeInfo::Nullable>
			{
				static Value BoxValue(Nullable<T> object, ITypeDescriptor* typeDescriptor)
				{
					return object?ValueAccessor<T, ITypeInfo::TypeDescriptor>::BoxValue(object.Value(), typeDescriptor):Value();
				}

				static Nullable<T> UnboxValue(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					if(value.IsNull())
					{
						return Nullable<T>();
					}
					else
					{
						return ValueAccessor<T, ITypeInfo::TypeDescriptor>::UnboxValue(value, typeDescriptor, valueName);
					}
				}
			};

			template<typename T>
			struct ValueAccessor<T, ITypeInfo::TypeDescriptor>
			{
				static Value BoxValue(const T& object, ITypeDescriptor* typeDescriptor)
				{
					using Type = std::remove_cvref_t<T>;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
					if(!typeDescriptor)
					{
						typeDescriptor = GetTypeDescriptor<Type>();
					}
#endif
					return Value::From(Ptr(new IValueType::TypedBox<Type>(object)), typeDescriptor);
				}

				static T UnboxValue(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					using Type = std::remove_cvref_t<T>;
					if (auto unboxedValue = value.GetBoxedValue().Cast<IValueType::TypedBox<Type>>())
					{
						return unboxedValue->value;
					}
					else
					{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
						if (!typeDescriptor)
						{
							typeDescriptor = GetTypeDescriptor<Type>();
						}
						throw ArgumentTypeMismtatchException(valueName, typeDescriptor, Value::BoxedValue, value);
#else
						CHECK_FAIL(L"vl::reflection::description::UnboxValue()#Argument type mismatch.");
#endif
					}
				}
			};

			template<>
			struct ValueAccessor<Value, ITypeInfo::TypeDescriptor>
			{
				static Value BoxValue(const Value& object, ITypeDescriptor* typeDescriptor)
				{
					return object;
				}

				static Value UnboxValue(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					return value;
				}
			};

			template<>
			struct ValueAccessor<VoidValue, ITypeInfo::TypeDescriptor>
			{
				static Value BoxValue(const VoidValue& object, ITypeDescriptor* typeDescriptor)
				{
					return Value();
				}

				static VoidValue UnboxValue(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					return VoidValue();
				}
			};
		}
	}
}

#endif
