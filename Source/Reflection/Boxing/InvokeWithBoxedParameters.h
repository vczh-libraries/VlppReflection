/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/
 
#ifndef VCZH_REFLECTION_BOXING_INVOKEWITHBOXEDPARAMETERS
#define VCZH_REFLECTION_BOXING_INVOKEWITHBOXEDPARAMETERS
 
#include "BoxingParameter.h"
 
namespace vl
{
	namespace reflection
	{
		namespace description
		{
/***********************************************************************
UnboxAndCall
***********************************************************************/
			
			namespace internal_helper
			{
				template<typename TFunction, typename ...TArgs>
				auto UnboxAndCallObject(TFunction& function, IMethodInfo* methodInfo, const Ptr<IValueReadonlyList>& arguments) -> decltype(function(std::declval<TArgs>()...))
				{
					// function(arguments)
					CHECK_FAIL(L"UnboxAndCallobject not implemented.");
				}

				template<typename TFunction, typename ...TArgs>
				auto UnboxAndCallFunction(TFunction function, IMethodInfo* methodInfo, collections::Array<Value>& arguments) -> decltype(function(std::declval<TArgs>()...))
				{
					// function(arguments)
					CHECK_FAIL(L"UnboxAndCallFunction not implemented.");
				}

				template<typename TClass, typename TFunction, typename ...TArgs>
				auto UnboxAndCallMethod(TFunction function, IMethodInfo* methodInfo, collections::Array<Value>& arguments, TClass* object) -> decltype((object->*function)(std::declval<TArgs>()...))
				{
					// (object->*function)(arguments)
					CHECK_FAIL(L"UnboxAndCallMethod not implemented.");
				}

				template<typename TClass, typename TFunction, typename ...TArgs>
				auto UnboxAndCallExternal(TFunction function, IMethodInfo* methodInfo, collections::Array<Value>& arguments, TClass* object) -> decltype(function(object, std::declval<TArgs>()...))
				{
					// function(object, arguments)
					CHECK_FAIL(L"UnboxAndCallExternal not implemented.");
				}

				template<typename TClass, typename R, typename ...TArgs>
				R UnboxAndNew(IMethodInfo* methodInfo, collections::Array<Value>& arguments)
				{
					// new TClass(arguments)
					CHECK_FAIL(L"UnboxAndNew not implemented.");
				}

				template<typename TClass, typename R, typename ...TArgs>
				Value InvokeMethod(TClass* object, R(__thiscall TClass::* method)(TArgs...), IMethodInfo* methodInfo, collections::Array<Value>& arguments)
				{
					using TFunction = R(__thiscall TClass::*)(TArgs...);
					if constexpr (std::is_same_v<R, void>)
					{
						UnboxAndCallMethod<TClass, TFunction, TArgs...>(method, methodInfo, arguments, object);
						return {};
					}
					else
					{
						auto td = methodInfo ? methodInfo->GetReturn()->GetTypeDescriptor() : nullptr;
						return BoxParameter(UnboxAndCallMethod<TClass, TFunction, TArgs...>(method, methodInfo, arguments, object), td);
					}
				}

				template<typename TClass, typename R, typename ...TArgs>
				Value InvokeExternal(TClass* object, R(*method)(TClass*, TArgs...), IMethodInfo* methodInfo, collections::Array<Value>& arguments)
				{
					using TFunction = R(*)(TClass*, TArgs...);
					if constexpr (std::is_same_v<R, void>)
					{
						UnboxAndCallExternal<TClass, TFunction, TArgs...>(method, methodInfo, arguments, object);
						return {};
					}
					else
					{
						auto td = methodInfo ? methodInfo->GetReturn()->GetTypeDescriptor() : nullptr;
						return BoxParameter(UnboxAndCallExternal<TClass, TFunction, TArgs...>(method, methodInfo, arguments, object), td);
					}
				}

				template<typename R, typename ...TArgs>
				Value InvokeFunction(R(*method)(TArgs...), MethodInfoImpl* methodInfo, collections::Array<Value>& arguments)
				{
					using TFunction = R(*)(TArgs...);
					if constexpr (std::is_same_v<R, void>)
					{
						UnboxAndCallFunction<TFunction, TArgs...>(method, methodInfo, arguments);
						return {};
					}
					else
					{
						auto td = methodInfo ? methodInfo->GetReturn()->GetTypeDescriptor() : nullptr;
						return BoxParameter(UnboxAndCallFunction<TFunction, TArgs...>(method, methodInfo, arguments), td);
					}
				}

				template<typename TFunction, typename ...TArgs>
				Value InvokeObject(TFunction& function, MethodInfoImpl* methodInfo, const Ptr<IValueReadonlyList>& arguments)
				{
					using TResult = decltype(UnboxAndCallObject<TFunction&, TArgs...>(function, methodInfo, arguments));
					if constexpr (std::is_same_v<TResult, void>)
					{
						UnboxAndCallObject<TFunction&, TArgs...>(function, methodInfo, arguments);
						return {};
					}
					else
					{
						auto td = methodInfo ? methodInfo->GetReturn()->GetTypeDescriptor() : nullptr;
						return BoxParameter(UnboxAndCallObject<TFunction&, TArgs...>(function, methodInfo, arguments), td);
					}
				}
			}
		}
	}
}

#endif