/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/
 
#ifndef VCZH_REFLECTION_METADATA_METADATA_EVENT
#define VCZH_REFLECTION_METADATA_METADATA_EVENT
 
#include "Metadata_Member.h"
 
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

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
 
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
							BoxParametersToList(arguments, std::forward<TArgs>(args)...);
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

				void InvokeInternal(DescriptableObject* thisObject, Ptr<IValueReadonlyList> arguments)override
				{
					TClass* object = UnboxValue<TClass*>(Value::From(thisObject), GetOwnerTypeDescriptor(), L"thisObject");
					Event<void(TArgs...)>& eventObject = object->*eventRef;
					internal_helper::InvokeObject<Event<void(TArgs...)>, TArgs...>(eventObject, nullptr, arguments);
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