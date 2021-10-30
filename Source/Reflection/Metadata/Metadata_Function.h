/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/
 
#ifndef VCZH_REFLECTION_METADATA_METADATA_FUNCTION
#define VCZH_REFLECTION_METADATA_METADATA_FUNCTION
 
#include "Metadata_Member.h"
 
namespace vl
{
	namespace reflection
	{
		namespace description
		{

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
 
/***********************************************************************
CustomConstructorInfoImpl<R(TArgs...)>
***********************************************************************/

			template<typename R, typename ...TArgs>
			class CustomConstructorInfoImpl<R(TArgs...)> : public MethodInfoImpl
			{
				using TClass = typename trait_helper::RemovePtr<std::remove_cvref_t<R>>::Type;
			protected:

				Value InvokeInternal(const Value& thisObject, collections::Array<Value>& arguments)override
				{
					return BoxParameter(unboxcall_helper::Unbox<MakeArgPacks<TArgs...>>::template AndNew<TClass, R>(this, arguments));
				}
 
				Value CreateFunctionProxyInternal(const Value& thisObject)override
				{
					Func<R(TArgs...)> proxy(
						LAMBDA([](TArgs ...args)->R
						{
							R result = new TClass(args...);
							return result;
						})
					);
					return BoxParameter(proxy);
				}
			public:
				CustomConstructorInfoImpl(const wchar_t* parameterNames[])
					:MethodInfoImpl(0, TypeInfoRetriver<R>::CreateTypeInfo(), true)
				{
					internal_helper::ConstructorArgumentAdder<TypeTuple<TArgs...>>::Add(this, parameterNames, 0);
				}

				IMethodInfo::ICpp* GetCpp()override
				{
					return nullptr;
				}
			};
 
/***********************************************************************
CustomMethodInfoImpl<TClass, R(TArgs...)>
CustomExternalMethodInfoImpl<TClass, R(TArgs...)>
***********************************************************************/

			class MethodInfoImpl_StaticCpp : public MethodInfoImpl, private IMethodInfo::ICpp
			{
			private:
				WString invokeTemplate;
				WString closureTemplate;

				const WString& GetInvokeTemplate()override
				{
					return invokeTemplate;
				}

				const WString& GetClosureTemplate()override
				{
					return closureTemplate;
				}
			public:
				MethodInfoImpl_StaticCpp(IMethodGroupInfo* _ownerMethodGroup, Ptr<ITypeInfo> _return, bool _isStatic, const wchar_t* _invokeTemplate, const wchar_t* _closureTemplate)
					:MethodInfoImpl(_ownerMethodGroup, _return, _isStatic)
				{
					CHECK_ERROR((_invokeTemplate == nullptr) == (_closureTemplate == nullptr), L"MethodInfoImpl_StaticCpp::MethodInfoImpl_StaticCpp()#Templates should all be set or default at the same time.");
					if (_invokeTemplate)
					{
						invokeTemplate = WString::Unmanaged(_invokeTemplate);
					}
					if (_closureTemplate)
					{
						closureTemplate = WString::Unmanaged(_closureTemplate);
					}
				}

				IMethodInfo::ICpp* GetCpp()override
				{
					return invokeTemplate.Length() == 0 || closureTemplate.Length() == 0 ? nullptr : this;
				}
			};

			template<typename TClass, typename R, typename ...TArgs>
			class CustomMethodInfoImpl<TClass, R(TArgs...)> : public MethodInfoImpl_StaticCpp
			{
			protected:
				R(__thiscall TClass::* method)(TArgs...);
 
				Value InvokeInternal(const Value& thisObject, collections::Array<Value>& arguments)override
				{
					TClass* object = UnboxValue<TClass*>(thisObject, GetOwnerTypeDescriptor(), L"thisObject");
					return invoke_helper::InvokeMethod<TClass, R, TArgs...>(object, method, this, arguments);
				}
 
				Value CreateFunctionProxyInternal(const Value& thisObject)override
				{
					TClass* object = UnboxValue<TClass*>(thisObject, GetOwnerTypeDescriptor(), L"thisObject");
					Func<R(TArgs...)> proxy(object, method);
					return BoxParameter(proxy);
				}
			public:
				CustomMethodInfoImpl(const wchar_t* parameterNames[], R(__thiscall TClass::* _method)(TArgs...), const wchar_t* _invokeTemplate, const wchar_t* _closureTemplate)
					:MethodInfoImpl_StaticCpp(0, TypeInfoRetriver<R>::CreateTypeInfo(), false, _invokeTemplate, _closureTemplate)
					, method(_method)
				{
					internal_helper::ConstructorArgumentAdder<TypeTuple<TArgs...>>::Add(this, parameterNames, 0);
				}
			};
 
			template<typename TClass, typename R, typename ...TArgs>
			class CustomExternalMethodInfoImpl<TClass, R(TArgs...)> : public MethodInfoImpl_StaticCpp
			{
			protected:
				R(*method)(TClass*, TArgs...);
 
				Value InvokeInternal(const Value& thisObject, collections::Array<Value>& arguments)override
				{
					TClass* object = UnboxValue<TClass*>(thisObject, GetOwnerTypeDescriptor(), L"thisObject");
					return invoke_helper::InvokeExternal<TClass, R, TArgs...>(object, method, this, arguments);
				}
 
				Value CreateFunctionProxyInternal(const Value& thisObject)override
				{
					TClass* object = UnboxValue<TClass*>(thisObject, GetOwnerTypeDescriptor(), L"thisObject");
					Func<R(TArgs...)> proxy = Curry(Func<R(TClass*, TArgs...)>(method))(object);
					return BoxParameter(proxy);
				}
			public:
				CustomExternalMethodInfoImpl(const wchar_t* parameterNames[], R(*_method)(TClass*, TArgs...), const wchar_t* _invokeTemplate, const wchar_t* _closureTemplate)
					:MethodInfoImpl_StaticCpp(0, TypeInfoRetriver<R>::CreateTypeInfo(), false, _invokeTemplate, _closureTemplate)
					, method(_method)
				{
					internal_helper::ConstructorArgumentAdder<TypeTuple<TArgs...>>::Add(this, parameterNames, 0);
				}
			};
 
/***********************************************************************
CustomStaticMethodInfoImpl<R(TArgs...)>
***********************************************************************/

			template<typename R, typename ...TArgs>
			class CustomStaticMethodInfoImpl<R(TArgs...)> : public MethodInfoImpl_StaticCpp
			{
			protected:
				R(* method)(TArgs...);
 
				Value InvokeInternal(const Value& thisObject, collections::Array<Value>& arguments)override
				{
					return invoke_helper::InvokeFunction<R, TArgs...>(method, this, arguments);
				}
 
				Value CreateFunctionProxyInternal(const Value& thisObject)override
				{
					Func<R(TArgs...)> proxy(method);
					return BoxParameter(proxy);
				}
			public:
				CustomStaticMethodInfoImpl(const wchar_t* parameterNames[], R(* _method)(TArgs...), const wchar_t* _invokeTemplate, const wchar_t* _closureTemplate)
					:MethodInfoImpl_StaticCpp(0, TypeInfoRetriver<R>::CreateTypeInfo(), true, _invokeTemplate, _closureTemplate)
					,method(_method)
				{
					internal_helper::ConstructorArgumentAdder<TypeTuple<TArgs...>>::Add(this, parameterNames, 0);
				}
			};
#endif
		}
	}
}

#endif