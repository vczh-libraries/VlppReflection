/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/
 
#ifndef VCZH_REFLECTION_WRAPPERS_FUNCTIONWRAPPERS
#define VCZH_REFLECTION_WRAPPERS_FUNCTIONWRAPPERS
 
#include "../Boxing/InvokeWithBoxedParameters.h"
 
namespace vl
{
	namespace reflection
	{
		namespace description
		{
/***********************************************************************
Function Wrappers
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

				Value Invoke(Ptr<IValueReadonlyList> arguments)override
				{
					if (!arguments || arguments->GetCount() != sizeof...(TArgs))
					{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
						throw ArgumentCountMismtatchException();
#else
						CHECK_FAIL(L"Argument count mismatch.");
#endif
					}
					return invoke_helper::InvokeObject<FunctionType, TArgs...>(function, nullptr, arguments);
				}
			};
		}
	}
}

#endif