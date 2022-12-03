/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_METADATA_METADATA_MEMBER
#define VCZH_REFLECTION_METADATA_METADATA_MEMBER

#include "Metadata.h"
#include "../Boxing/Boxing.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{ 
/***********************************************************************
ConstructorArgumentAdder
***********************************************************************/

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			
			namespace internal_helper
			{
				template<typename T>
				struct ConstructorArgumentAdder
				{
					static void Add(MethodInfoImpl* methodInfo, const wchar_t* parameterNames[], vint index)
					{
					}
				};

				template<typename T0, typename ...TNextArgs>
				struct ConstructorArgumentAdder<TypeTuple<T0, TNextArgs...>>
				{
					static void Add(MethodInfoImpl* methodInfo, const wchar_t* parameterNames[], vint index)
					{
						methodInfo->AddParameter(Ptr(new ParameterInfoImpl(methodInfo, parameterNames[index], TypeInfoRetriver<T0>::CreateTypeInfo())));
						ConstructorArgumentAdder<TypeTuple<TNextArgs...>>::Add(methodInfo, parameterNames, index + 1);
					}
				};
			}
 
/***********************************************************************
MethodInfoImpl
***********************************************************************/
 
			template<typename T>
			class CustomConstructorInfoImpl{};
 
			template<typename TClass, typename T>
			class CustomMethodInfoImpl{};
 
			template<typename TClass, typename T>
			class CustomExternalMethodInfoImpl{};
 
			template<typename T>
			class CustomStaticMethodInfoImpl{};

			template<typename TClass, typename T>
			class CustomEventInfoImpl{};

#endif
		}
	}
}

#endif
