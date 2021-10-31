/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REFLECTION_BOXING_BOXINGPARAMETER
#define VCZH_REFLECTION_BOXING_BOXINGPARAMETER

#include "BoxingValue.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
/***********************************************************************
BoxParameter, UnboxParameter
***********************************************************************/

			template<typename T, TypeFlags Flag>
			struct ParameterAccessor
			{
			};
			
			/// <summary>Box an reflectable object. It supports generic types such as containers, functions (should be Func&lt;T&gt;), etc.</summary>
			/// <returns>The boxed value.</returns>
			/// <typeparam name="T">Type of the object.</typeparam>
			/// <param name="object">The object to box.</param>
			/// <param name="typeDescriptor">The type descriptor of the object (optional).</param>
			template<typename T>
			Value BoxParameter(T&& object, ITypeDescriptor* typeDescriptor)
			{
				using TIR = TypeInfoRetriver<std::remove_reference_t<T>>;
				return ParameterAccessor<std::remove_reference_t<T>, TIR::TypeFlag>::BoxParameter(object, typeDescriptor);
			}
			
			/// <summary>Box an reflectable object. It supports generic types such as containers, functions (should be Func&lt;T&gt;), etc.</summary>
			/// <typeparam name="T">Type of the object.</typeparam>
			/// <returns>The unboxed object. It could be the same object that is boxed, or it could be a new object.</returns>
			/// <param name="value">The value to unbox.</param>
			/// <param name="typeDescriptor">The type descriptor of the object (optional).</param>
			/// <param name="valueName">The name of the object to provide a friendly exception message if the conversion is failed (optional).</param>
			template<typename T>
			Unboxed<T> UnboxParameter(const Value& value, ITypeDescriptor* typeDescriptor , const WString& valueName)
			{
				using TIR = TypeInfoRetriver<std::remove_reference_t<T>>;
				return ParameterAccessor<T, TIR::TypeFlag>::UnboxParameter(value, typeDescriptor, valueName);
			}

/***********************************************************************
BoxParametersToList
***********************************************************************/

			inline void BoxParametersToList(Ptr<IValueList> arguments) {}

			template<typename T0, typename ...TArgs>
			void BoxParametersToList(Ptr<IValueList> arguments, T0&& p0, TArgs&& ...args)
			{
				arguments->Add(description::BoxParameter(p0));
				BoxParametersToList(arguments, args...);
			}

			class Value_xs
			{
			protected:
				collections::Array<Value>	arguments;
			public:
				Value_xs()
				{
				}

				template<typename T>
				Value_xs& operator,(T& value)
				{
					arguments.Resize(arguments.Count() + 1);
					arguments[arguments.Count() - 1] = BoxParameter(value);
					return *this;
				}

				Value_xs& operator,(const Value& value)
				{
					arguments.Resize(arguments.Count() + 1);
					arguments[arguments.Count() - 1] = value;
					return *this;
				}

				operator collections::Array<Value>& ()
				{
					return arguments;
				}
			};
 
/***********************************************************************
Basic Types
***********************************************************************/

			template<typename T>
			struct ParameterAccessor<T, TypeFlags::NonGenericType>
			{
				static Value BoxParameter(const T& object, ITypeDescriptor* typeDescriptor)
				{
					return BoxValue<T>(object, typeDescriptor);
				}

				static Unboxed<T> UnboxParameter(const Value& value, ITypeDescriptor* typeDescriptor, const WString& valueName)
				{
					return { new T(std::move(UnboxValue<T>(value, typeDescriptor, valueName))), true };
				}
			};
		}
	}
}

#endif
