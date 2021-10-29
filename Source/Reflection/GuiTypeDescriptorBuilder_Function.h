/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/
 
#ifndef VCZH_REFLECTION_GUITYPEDESCRIPTORBUILDER_FUNCTION
#define VCZH_REFLECTION_GUITYPEDESCRIPTORBUILDER_FUNCTION
 
#include "GuiTypeDescriptorBuilder.h"
 
namespace vl
{
	namespace reflection
	{
		namespace description
		{
			template<typename ...TArgs>
			struct EventHelper
			{
				using Handler = const Func<void(TArgs...)>&;

				class EventHandlerImpl : public Object, public reflection::description::IEventHandler
				{
				public:
					Ptr<EventHandler> handler;

					EventHandlerImpl(Ptr<EventHandler> _handler)
						:handler(_handler)
					{
					}

					bool IsAttached()override
					{
						return handler->IsAttached();
					}
				};

				static Ptr<reflection::description::IEventHandler> Attach(Event<void(TArgs...)>& e, Handler handler)
				{
					return MakePtr<EventHandlerImpl>(e.Add(handler));
				}

				static bool Detach(Event<void(TArgs...)>& e, Ptr<reflection::description::IEventHandler> handler)
				{
					auto impl = handler.Cast<EventHandlerImpl>();
					if (!impl) return false;
					return e.Remove(impl->handler);
				}

				static Event<void(TArgs...)>& Invoke(Event<void(TArgs...)>& e)
				{
					return e;
				}
			};

/***********************************************************************
DetailTypeInfoRetriver<Func<R(TArgs...)>>
***********************************************************************/

#ifndef VCZH_DEBUG_NO_REFLECTION
			namespace internal_helper
			{
				template<typename T>
				struct GenericArgumentAdder
				{
					static void Add(Ptr<GenericTypeInfo> genericType)
					{
					}
				};

				template<typename T0, typename ...TNextArgs>
				struct GenericArgumentAdder<TypeTuple<T0, TNextArgs...>>
				{
					static void Add(Ptr<GenericTypeInfo> genericType)
					{
						genericType->AddGenericArgument(TypeInfoRetriver<T0>::CreateTypeInfo());
						GenericArgumentAdder<TypeTuple<TNextArgs...>>::Add(genericType);
					}
				};
			}
#endif

			template<typename R, typename ...TArgs>
			struct DetailTypeInfoRetriver<Func<R(TArgs...)>, TypeFlags::FunctionType>
			{
				typedef DetailTypeInfoRetriver<const Func<R(TArgs...)>, TypeFlags::NonGenericType>	UpLevelRetriver;
 
				static const ITypeInfo::Decorator								Decorator=UpLevelRetriver::Decorator;
				typedef IValueList												Type;
				typedef typename UpLevelRetriver::TempValueType					TempValueType;
				typedef typename UpLevelRetriver::ResultReferenceType			ResultReferenceType;
				typedef typename UpLevelRetriver::ResultNonReferenceType		ResultNonReferenceType;
 
#ifndef VCZH_DEBUG_NO_REFLECTION
				static Ptr<ITypeInfo> CreateTypeInfo(TypeInfoHint hint)
				{
					auto functionType = MakePtr<TypeDescriptorTypeInfo>(GetTypeDescriptor<IValueFunctionProxy>(), hint);
 
					auto genericType = MakePtr<GenericTypeInfo>(functionType);
					genericType->AddGenericArgument(TypeInfoRetriver<R>::CreateTypeInfo());
					internal_helper::GenericArgumentAdder<TypeTuple<TArgs...>>::Add(genericType);

					auto type = MakePtr<SharedPtrTypeInfo>(genericType);
					return type;
				}
#endif
			};

			template<typename R, typename ...TArgs>
			struct DetailTypeInfoRetriver<const Func<R(TArgs...)>, TypeFlags::FunctionType>
				: DetailTypeInfoRetriver<Func<R(TArgs...)>, TypeFlags::FunctionType>
			{
			};
 
/***********************************************************************
UnboxAndCall
***********************************************************************/
			
			namespace internal_helper
			{
				template<typename ...TArgs, typename TFunction>
				auto UnboxAndCall(TFunction&& function, IMethodInfo* methodInfo, const Ptr<IValueList>& arguments) -> decltype(function(std::declval<TArgs>()...))
				{
					throw 0;
				}

				template<typename TClass, typename ...TArgs, typename TFunction>
				auto UnboxAndCallExternal(TFunction&& function, IMethodInfo* methodInfo, const Ptr<IValueList>& arguments, TClass* object) -> decltype(function(object, std::declval<TArgs>()...))
				{
					throw 0;
				}

				template<typename TClass, typename ...TArgs>
				TClass* UnboxAndNew(IMethodInfo* methodInfo, const Ptr<IValueList>& arguments)
				{
					throw 0;
				}

				template<typename TClass, typename R, typename ...TArgs>
				Value InvokeMethod(TClass* object, R(__thiscall TClass::* method)(TArgs...), MethodInfoImpl* methodInfo, collections::Array<Value>& arguments)
				{
					if constexpr (std::is_same_v<R, void>)
					{
						UnboxAndCall<TArgs...>(object->*method, methodInfo, arguments);
						return {};
					}
					else
					{
						return BoxParameter(UnboxAndCall<TArgs...>(object->*method, methodInfo, arguments), methodInfo->GetReturn()->GetTypeDescriptor());
					}
				}

				template<typename TClass, typename R, typename ...TArgs>
				Value InvokeExternal(TClass* object, R(method)(TArgs...), MethodInfoImpl* methodInfo, collections::Array<Value>& arguments)
				{
					if constexpr (std::is_same_v<R, void>)
					{
						UnboxAndCallExternal<TClass, TArgs...>(method, methodInfo, arguments, object);
						return {};
					}
					else
					{
						return BoxParameter(UnboxAndCallExternal<TClass, TArgs...>(method, methodInfo, arguments, object), methodInfo->GetReturn()->GetTypeDescriptor());
					}
				}

				template<typename R, typename ...TArgs>
				Value InvokeFunction(R(*method)(TArgs...), MethodInfoImpl* methodInfo, collections::Array<Value>& arguments)
				{
					if constexpr (std::is_same_v<R, void>)
					{
						UnboxAndCall<TArgs...>(method, methodInfo, arguments);
						return {};
					}
					else
					{
						return BoxParameter(UnboxAndCall<TArgs...>(method, methodInfo, arguments), methodInfo->GetReturn()->GetTypeDescriptor());
					}
				}
			}
 
/***********************************************************************
AddValueToList
***********************************************************************/
			
			namespace internal_helper
			{
				extern void AddValueToList(Ptr<IValueList> arguments);

				template<typename T0, typename ...TArgs>
				void AddValueToList(Ptr<IValueList> arguments, T0&& p0, TArgs&& ...args)
				{
					arguments->Add(description::BoxParameter(p0));
					AddValueToList(arguments, args...);
				}
			}
 
/***********************************************************************
Argument Adders
***********************************************************************/
			
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
						methodInfo->AddParameter(new ParameterInfoImpl(methodInfo, parameterNames[index], TypeInfoRetriver<T0>::CreateTypeInfo()));
						ConstructorArgumentAdder<TypeTuple<TNextArgs...>>::Add(methodInfo, parameterNames, index + 1);
					}
				};
			}
 
/***********************************************************************
ValueFunctionProxyWrapper<Func<R(TArgs...)>>
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
					return BoxParameter(internal_helper::UnboxAndCall<TArgs...>(function, nullptr, arguments));
				}
			};
 
/***********************************************************************
ParameterAccessor<Func<R(TArgs...)>>
***********************************************************************/
 
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
								internal_helper::AddValueToList(arguments, std::forward<TArgs>(args)...);
								typedef typename TypeInfoRetriver<R>::TempValueType ResultType;
								ResultType proxyResult;
								description::UnboxParameter(functionProxy->Invoke(arguments), proxyResult);
								return proxyResult;
							}), true };
						}
					}
				}
			};
 
			template<typename R, typename ...TArgs>
			struct ParameterAccessor<const Func<R(TArgs...)>, TypeFlags::FunctionType> : ParameterAccessor<Func<R(TArgs...)>, TypeFlags::FunctionType>
			{
			};

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
 
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
 
/***********************************************************************
CustomConstructorInfoImpl<R(TArgs...)>
***********************************************************************/

			template<typename R, typename ...TArgs>
			class CustomConstructorInfoImpl<R(TArgs...)> : public MethodInfoImpl
			{
			protected:
				Value InvokeInternal(const Value& thisObject, collections::Array<Value>& arguments)override
				{
					using TClass = typename TypeInfoRetriver<R>::Type;
					return BoxParameter(internal_helper::UnboxAndNew<TClass, TArgs...>(this, arguments));
				}
 
				Value CreateFunctionProxyInternal(const Value& thisObject)override
				{
					Func<R(TArgs...)> proxy(
						LAMBDA([](TArgs ...args)->R
						{
							R result = new typename TypeInfoRetriver<R>::Type(args...);
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
CustomStaticMethodInfoImpl<TClass, R(TArgs...)>
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
					return internal_helper::InvokeMethod<TClass, R, TArgs...>(object, method, this, arguments);
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
					return internal_helper::InvokeExternal<TClass, R, TArgs...>(object, method, this, arguments);
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
					return interhal_helper::InvokeFunction<R, TArgs...>(method, this, arguments);
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
 
/***********************************************************************
CustomEventInfoImpl<void(TArgs...)>
***********************************************************************/

			template<typename TClass, typename ...TArgs>
			class CustomEventInfoImpl<TClass, void(TArgs...)> : public EventInfoImpl
			{
			protected:
				Event<void(TArgs...)> TClass::*			eventRef;

				Ptr<IEventHandler> AttachInternal(DescriptableObject* thisObject, Ptr<IValueFunctionProxy> handler)override
				{
					TClass* object = UnboxValue<TClass*>(Value::From(thisObject), GetOwnerTypeDescriptor(), L"thisObject");
					Event<void(TArgs...)>& eventObject = object->*eventRef;
					auto func = Func<void(TArgs...)>([=](TArgs ...args)
						{
							auto arguments = IValueList::Create();
							internal_helper::AddValueToList(arguments, std::forward<TArgs>(args)...);
							handler->Invoke(arguments);
						});
					return EventHelper<TArgs...>::Attach(eventObject, func);
				}

				bool DetachInternal(DescriptableObject* thisObject, Ptr<IEventHandler> handler)override
				{
					TClass* object = UnboxValue<TClass*>(Value::From(thisObject), GetOwnerTypeDescriptor(), L"thisObject");
					Event<void(TArgs...)>& eventObject = object->*eventRef;
					return EventHelper<TArgs...>::Detach(eventObject, handler);
				}

				void InvokeInternal(DescriptableObject* thisObject, Ptr<IValueList> arguments)override
				{
					TClass* object = UnboxValue<TClass*>(Value::From(thisObject), GetOwnerTypeDescriptor(), L"thisObject");
					Event<void(TArgs...)>& eventObject = object->*eventRef;
					internal_helper::UnboxAndCall<TArgs...>(eventObject, nullptr, arguments);
				}

				Ptr<ITypeInfo> GetHandlerTypeInternal()override
				{
					return TypeInfoRetriver<Func<void(TArgs...)>>::CreateTypeInfo();
				}
			public:
				CustomEventInfoImpl(ITypeDescriptor* _ownerTypeDescriptor, const WString& _name, Event<void(TArgs...)> TClass::* _eventRef)
					:EventInfoImpl(_ownerTypeDescriptor, _name)
					, eventRef(_eventRef)
				{
				}

				~CustomEventInfoImpl()
				{
				}

				IEventInfo::ICpp* GetCpp()override
				{
					return nullptr;
				}
			};

			template<typename T>
			struct CustomEventFunctionTypeRetriver
			{
				typedef vint								Type;
			};

			template<typename TClass, typename TEvent>
			struct CustomEventFunctionTypeRetriver<Event<TEvent> TClass::*>
			{
				typedef TEvent								Type;
			};
#endif
		}
	}
}

#endif