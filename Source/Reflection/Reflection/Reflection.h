/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_REFLECTION_REFLECTION
#define VCZH_REFLECTION_REFLECTION_REFLECTION

#include "Macros.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{

/***********************************************************************
Predefined Types
***********************************************************************/

#define REFLECTION_PREDEFINED_PRIMITIVE_TYPES(F)\
			F(vuint8_t)		\
			F(vuint16_t)	\
			F(vuint32_t)	\
			F(vuint64_t)	\
			F(vint8_t)		\
			F(vint16_t)		\
			F(vint32_t)		\
			F(vint64_t)		\
			F(float)		\
			F(double)		\
			F(bool)			\
			F(wchar_t)		\
			F(WString)		\
			F(Locale)		\

#define REFLECTION_PREDEFINED_SERIALIZABLE_TYPES(F)\
			REFLECTION_PREDEFINED_PRIMITIVE_TYPES(F)	\
			F(DateTime)									\

#ifndef VCZH_DEBUG_NO_REFLECTION

#define REFLECTION_PREDEFINED_COMPLEX_TYPES(F, VOID_TYPE)\
			F(VOID_TYPE)					\
			F(VoidValue)					\
			F(IDescriptable)				\
			F(DescriptableObject)			\
			F(DateTime)						\
			F(IValueEnumerator)				\
			F(IValueEnumerable)				\
			F(IValueReadonlyList)			\
			F(IValueArray)					\
			F(IValueList)					\
			F(IValueObservableList)			\
			F(IValueReadonlyDictionary)		\
			F(IValueDictionary)				\
			F(IValueInterfaceProxy)			\
			F(IValueFunctionProxy)			\
			F(IValueSubscription)			\
			F(IValueCallStack)				\
			F(IValueException)				\
			F(IBoxedValue)					\
			F(IBoxedValue::CompareResult)	\
			F(IValueType)					\
			F(IEnumType)					\
			F(ISerializableType)			\
			F(ITypeInfo)					\
			F(ITypeInfo::Decorator)			\
			F(IMemberInfo)					\
			F(IEventHandler)				\
			F(IEventInfo)					\
			F(IPropertyInfo)				\
			F(IParameterInfo)				\
			F(IMethodInfo)					\
			F(IMethodGroupInfo)				\
			F(TypeDescriptorFlags)			\
			F(ITypeDescriptor)				\

			DECL_TYPE_INFO(Value)
			REFLECTION_PREDEFINED_PRIMITIVE_TYPES(DECL_TYPE_INFO)
			REFLECTION_PREDEFINED_COMPLEX_TYPES(DECL_TYPE_INFO, void)

#endif

#define DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER(TYPENAME)\
			template<>\
			struct TypedValueSerializerProvider<TYPENAME>\
			{\
				static TYPENAME GetDefaultValue();\
				static bool Serialize(const TYPENAME& input, WString& output);\
				static bool Deserialize(const WString& input, TYPENAME& output);\
			};\

			REFLECTION_PREDEFINED_PRIMITIVE_TYPES(DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER)

#undef DEFINE_TYPED_VALUE_SERIALIZER_PROVIDER

/***********************************************************************
Interface Implementation Proxy (Implement)
***********************************************************************/

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

#pragma warning(push)
#pragma warning(disable:4250)
			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(IValueEnumerator)
				Value GetCurrent()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetCurrent);
				}

				vint GetIndex()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetIndex);
				}

				bool Next()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(Next);
				}
			END_INTERFACE_PROXY(IValueEnumerator)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(IValueEnumerable)
				Ptr<IValueEnumerator> CreateEnumerator()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(CreateEnumerator);
				}
			END_INTERFACE_PROXY(IValueEnumerable)

			BEGIN_INTERFACE_PROXY_SHAREDPTR(IValueReadonlyList, IValueEnumerable)
				vint GetCount()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetCount);
				}

				Value Get(vint index)override
				{
					INVOKEGET_INTERFACE_PROXY(Get, index);
				}

				bool Contains(const Value& value)override
				{
					INVOKEGET_INTERFACE_PROXY(Contains, value);
				}

				vint IndexOf(const Value& value)override
				{
					INVOKEGET_INTERFACE_PROXY(IndexOf, value);
				}
			END_INTERFACE_PROXY(IValueReadonlyList)

			BEGIN_INTERFACE_PROXY_SHAREDPTR(IValueArray, IValueReadonlyList)
				void Set(vint index, const Value& value)override
				{
					INVOKE_INTERFACE_PROXY(Set, index, value);
				}

				void Resize(vint size)override
				{
					INVOKE_INTERFACE_PROXY(Resize, size);
				}
			END_INTERFACE_PROXY(IValueList)

			BEGIN_INTERFACE_PROXY_SHAREDPTR(IValueList, IValueReadonlyList)
				void Set(vint index, const Value& value)override
				{
					INVOKE_INTERFACE_PROXY(Set, index, value);
				}

				vint Add(const Value& value)override
				{
					INVOKEGET_INTERFACE_PROXY(Add, value);
				}

				vint Insert(vint index, const Value& value)override
				{
					INVOKEGET_INTERFACE_PROXY(Insert, index, value);
				}

				bool Remove(const Value& value)override
				{
					INVOKEGET_INTERFACE_PROXY(Remove, value);
				}

				bool RemoveAt(vint index)override
				{
					INVOKEGET_INTERFACE_PROXY(RemoveAt, index);
				}

				void Clear()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(Clear);
				}
			END_INTERFACE_PROXY(IValueList)

			BEGIN_INTERFACE_PROXY_SHAREDPTR(IValueObservableList, IValueList)
			END_INTERFACE_PROXY(IValueObservableList)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(IValueReadonlyDictionary)
				Ptr<IValueReadonlyList> GetKeys()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetKeys);
				}

				Ptr<IValueReadonlyList> GetValues()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetValues);
				}

				vint GetCount()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetCount);
				}

				Value Get(const Value& key)override
				{
					INVOKEGET_INTERFACE_PROXY(Get, key);
				}
			END_INTERFACE_PROXY(IValueReadonlyDictionary)

			BEGIN_INTERFACE_PROXY_SHAREDPTR(IValueDictionary, IValueReadonlyDictionary)
				void Set(const Value& key, const Value& value)override
				{
					INVOKE_INTERFACE_PROXY(Set, key, value);
				}

				bool Remove(const Value& key)override
				{
					INVOKEGET_INTERFACE_PROXY(Remove, key);
				}

				void Clear()override
				{
					INVOKE_INTERFACE_PROXY_NOPARAMS(Clear);
				}
			END_INTERFACE_PROXY(IValueDictionary)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(IValueSubscription)
				bool Open()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(Open);
				}

				bool Update()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(Update);
				}

				bool Close()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(Close);
				}
			END_INTERFACE_PROXY(IValueSubscription)

#pragma warning(pop)

#endif

/***********************************************************************
Helper Functions
***********************************************************************/

			extern vint										ITypeDescriptor_GetTypeDescriptorCount();
			extern ITypeDescriptor*							ITypeDescriptor_GetTypeDescriptor(vint index);
			extern ITypeDescriptor*							ITypeDescriptor_GetTypeDescriptor(const WString& name);
			extern ITypeDescriptor*							ITypeDescriptor_GetTypeDescriptor(const Value& value);

/***********************************************************************
LoadPredefinedTypes
***********************************************************************/

			/// <summary>Register all reflectable types in <b>VlppReflection</b>.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			extern bool										LoadPredefinedTypes();
		}
	}
}

#endif
