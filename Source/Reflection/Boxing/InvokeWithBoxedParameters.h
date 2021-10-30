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

			namespace unboxcall_helper
			{
				template<typename T, vint I>
				struct ArgPack
				{
					using TArg = T;
					static const vint Index = I;
				};

				template<typename ...TArgs>
				struct ArgPacks
				{
				};

				template<typename TArgPacks, typename ...TArgs>
				struct MakeArgPacks_;

				template<typename ...TPacked>
				struct MakeArgPacks_<ArgPacks<TPacked...>>
				{
					using Type = ArgPacks<TPacked...>;
				};

				template<typename ...TPacked, typename T0, typename ...TArgs>
				struct MakeArgPacks_<ArgPacks<TPacked...>, T0, TArgs...>
				{
					using Type = typename MakeArgPacks_<ArgPacks<TPacked..., ArgPack<T0, sizeof...(TPacked)>>, TArgs...>::Type;
				};
			}

			template<typename ...TArgs>
			using MakeArgPacks = typename unboxcall_helper::MakeArgPacks_<unboxcall_helper::ArgPacks<>, TArgs...>::Type;

			namespace unboxcall_helper
			{
				template<typename TArgPacks>
				struct Unbox;

				template<typename ...TArgPacks>
				struct Unbox<ArgPacks<TArgPacks...>>
				{
					template<typename TFunction>
					static auto AndCallObject(TFunction&& function, IMethodInfo* methodInfo, const Ptr<IValueReadonlyList>& arguments) -> decltype(function(std::declval<typename TArgPacks::TArg>()...))
					{
						// function(arguments)
						return function(UnboxParameter<std::remove_cvref_t<typename TArgPacks::TArg>>(
							arguments->Get(TArgPacks::Index),
							(methodInfo ? methodInfo->GetParameter(TArgPacks::Index)->GetType()->GetTypeDescriptor() : nullptr)
							).Ref()...);
					}

					template<typename TFunction>
					static auto AndCallFunction(TFunction function, IMethodInfo* methodInfo, collections::Array<Value>& arguments) -> decltype(function(std::declval<typename TArgPacks::TArg>()...))
					{
						// function(arguments)
						return function(UnboxParameter<std::remove_cvref_t<typename TArgPacks::TArg>>(
							arguments[TArgPacks::Index],
							(methodInfo ? methodInfo->GetParameter(TArgPacks::Index)->GetType()->GetTypeDescriptor() : nullptr)
							).Ref()...);
					}

					template<typename TClass, typename TFunction>
					static auto AndCallMethod(TFunction function, IMethodInfo* methodInfo, collections::Array<Value>& arguments, TClass* object) -> decltype((object->*function)(std::declval<typename TArgPacks::TArg>()...))
					{
						// (object->*function)(arguments)
						return (object->*function)(UnboxParameter<std::remove_cvref_t<typename TArgPacks::TArg>>(
							arguments[TArgPacks::Index],
							(methodInfo ? methodInfo->GetParameter(TArgPacks::Index)->GetType()->GetTypeDescriptor() : nullptr)
							).Ref()...);
					}

					template<typename TClass, typename TFunction>
					static auto AndCallExternal(TFunction function, IMethodInfo* methodInfo, collections::Array<Value>& arguments, TClass* object) -> decltype(function(object, std::declval<typename TArgPacks::TArg>()...))
					{
						// function(object, arguments)
						return function(
							object,
							UnboxParameter<std::remove_cvref_t<typename TArgPacks::TArg>>(
								arguments[TArgPacks::Index],
								(methodInfo ? methodInfo->GetParameter(TArgPacks::Index)->GetType()->GetTypeDescriptor() : nullptr)
								).Ref()...
							);
					}

					template<typename TClass, typename R>
					static R AndNew(IMethodInfo* methodInfo, collections::Array<Value>& arguments)
					{
						// new TClass(arguments)
						return R(new TClass(UnboxParameter<std::remove_cvref_t<typename TArgPacks::TArg>>(
							arguments[TArgPacks::Index],
							(methodInfo ? methodInfo->GetParameter(TArgPacks::Index)->GetType()->GetTypeDescriptor() : nullptr)
							).Ref()...));
					}
				};
			}

/***********************************************************************
Invoke
***********************************************************************/

			namespace invoke_helper
			{
				template<typename TClass, typename R, typename ...TArgs>
				Value InvokeMethod(TClass* object, R(__thiscall TClass::* method)(TArgs...), IMethodInfo* methodInfo, collections::Array<Value>& arguments)
				{
					using TArgPacks = MakeArgPacks<TArgs...>;
					if constexpr (std::is_same_v<R, void>)
					{
						unboxcall_helper::Unbox<TArgPacks>::AndCallMethod(method, methodInfo, arguments, object);
						return {};
					}
					else
					{
						auto td = methodInfo ? methodInfo->GetReturn()->GetTypeDescriptor() : nullptr;
						return BoxParameter(unboxcall_helper::Unbox<TArgPacks>::AndCallMethod(method, methodInfo, arguments, object), td);
					}
				}

				template<typename TClass, typename R, typename ...TArgs>
				Value InvokeExternal(TClass* object, R(*method)(TClass*, TArgs...), IMethodInfo* methodInfo, collections::Array<Value>& arguments)
				{
					using TArgPacks = MakeArgPacks<TArgs...>;
					if constexpr (std::is_same_v<R, void>)
					{
						unboxcall_helper::Unbox<TArgPacks>::AndCallExternal(method, methodInfo, arguments, object);
						return {};
					}
					else
					{
						auto td = methodInfo ? methodInfo->GetReturn()->GetTypeDescriptor() : nullptr;
						return BoxParameter(unboxcall_helper::Unbox<TArgPacks>::AndCallExternal(method, methodInfo, arguments, object), td);
					}
				}

				template<typename R, typename ...TArgs>
				Value InvokeFunction(R(*method)(TArgs...), MethodInfoImpl* methodInfo, collections::Array<Value>& arguments)
				{
					using TArgPacks = MakeArgPacks<TArgs...>;
					if constexpr (std::is_same_v<R, void>)
					{
						unboxcall_helper::Unbox<TArgPacks>::AndCallFunction(method, methodInfo, arguments);
						return {};
					}
					else
					{
						auto td = methodInfo ? methodInfo->GetReturn()->GetTypeDescriptor() : nullptr;
						return BoxParameter(unboxcall_helper::Unbox<TArgPacks>::AndCallFunction(method, methodInfo, arguments), td);
					}
				}

				template<typename TFunction, typename ...TArgs>
				Value InvokeObject(TFunction& function, MethodInfoImpl* methodInfo, const Ptr<IValueReadonlyList>& arguments)
				{
					using TArgPacks = MakeArgPacks<TArgs...>;
					using TResult = decltype(unboxcall_helper::Unbox<TArgPacks>::AndCallObject(function, methodInfo, arguments));
					if constexpr (std::is_same_v<TResult, void>)
					{
						unboxcall_helper::Unbox<TArgPacks>::AndCallObject(function, methodInfo, arguments);
						return {};
					}
					else
					{
						auto td = methodInfo ? methodInfo->GetReturn()->GetTypeDescriptor() : nullptr;
						return BoxParameter(unboxcall_helper::Unbox<TArgPacks>::AndCallObject(function, methodInfo, arguments), td);
					}
				}
			}
		}
	}
}

#endif