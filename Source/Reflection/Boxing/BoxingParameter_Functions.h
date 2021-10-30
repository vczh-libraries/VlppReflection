/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_BOXING_BOXINGPARAMETER_FUNCTIONS
#define VCZH_REFLECTION_BOXING_BOXINGPARAMETER_FUNCTIONS

#include "InvokeWithBoxedParameters.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
/***********************************************************************
Functions
***********************************************************************/

			template<typename T>
			class ValueFunctionProxyWrapper
			{
			};

			template<typename R, typename ...TArgs>
			class ValueFunctionProxyWrapper<R(TArgs...)> : public Object, public virtual IValueFunctionProxy
			{
				typedef Func<R(TArgs...)>					FunctionType;
			protected:
				FunctionType			function;

			public:
				ValueFunctionProxyWrapper(const FunctionType& _function)
					:function(_function)
				{
				}
 
				FunctionType GetFunction()
				{
					return function;
				}
 
				Value Invoke(Ptr<IValueList> arguments)override
				{
					if (!arguments || arguments->GetCount() != sizeof...(TArgs))
					{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
						throw ArgumentCountMismtatchException();
#else
						CHECK_FAIL(L"Argument count mismatch.");
#endif
					}
					if constexpr (std::is_same_v<R, void>)
					{
						internal_helper::UnboxAndCallObject<FunctionType, TArgs...>(function, nullptr, arguments);
						return {};
					}
					else
					{
						return BoxParameter(internal_helper::UnboxAndCallObject<FunctionType, TArgs...>(function, nullptr, arguments));
					}
				}
			};
 
			template<typename R, typename ...TArgs>
			struct ParameterAccessor<Func<R(TArgs...)>, TypeFlags::FunctionType>
			{
				static Value BoxParameter(const Func<R(TArgs...)>& object, ITypeDescriptor* typeDescriptor)
				{
					typedef R(RawFunctionType)(TArgs...);
					Ptr<IValueFunctionProxy> result=new ValueFunctionProxyWrapper<RawFunctionType>(object);

					ITypeDescriptor* td = nullptr;
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
					td = Description<IValueFunctionProxy>::GetAssociatedTypeDescriptor();
#endif
					return BoxValue<Ptr<IValueFunctionProxy>>(result, td);
				}
 
				static Unboxed<Func<R(TArgs...)>> UnboxParameter(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					typedef R(RawFunctionType)(TArgs...);
					typedef Func<R(TArgs...)> FunctionType;
					typedef ValueFunctionProxyWrapper<RawFunctionType> ProxyType;
					Ptr<IValueFunctionProxy> functionProxy = UnboxValue<Ptr<IValueFunctionProxy>>(value, typeDescriptor, valueName);
					if (functionProxy)
					{
						if (auto proxy = functionProxy.Cast<ProxyType>())
						{
							return { new FunctionType(std::move(proxy->GetFunction())), true };
						}
						else
						{
							return { new FunctionType([functionProxy](TArgs ...args)
							{
								Ptr<IValueList> arguments = IValueList::Create();
								BoxParametersToList(arguments, std::forward<TArgs>(args)...);
								auto result = functionProxy->Invoke(arguments);
								if constexpr (!std::is_same_v<R, void>)
								{
									auto unboxed = description::UnboxParameter<std::remove_cvref_t<R>>(result);
									if (std::is_reference_v<R>)
									{
										CHECK_ERROR(!unboxed.IsOwned(), L"It is impossible to return a reference from a unboxed value, when the unboxing has to call new T(...).");
									}
									return unboxed.Ref();
								}
							}), true };
						}
					}
				}
			};
 
			template<typename R, typename ...TArgs>
			struct ParameterAccessor<const Func<R(TArgs...)>, TypeFlags::FunctionType> : ParameterAccessor<Func<R(TArgs...)>, TypeFlags::FunctionType>
			{
			};
		}
	}
}

#endif
