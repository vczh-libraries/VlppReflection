/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "GuiTypeDescriptorReflection.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			using namespace collections;

/***********************************************************************
Context
***********************************************************************/

			struct MetaonlyWriterContext
			{
				Dictionary<ITypeDescriptor*, vint>		tdIndex;
				Dictionary<IMethodInfo*, vint>			miIndex;
				Dictionary<IPropertyInfo*, vint>		piIndex;
				Dictionary<IEventInfo*, vint>			eiIndex;
			};

			struct MetaonlyReaderContext
			{
				Dictionary<WString, Ptr<ISerializableType>>		serializableTypes;
				List<Ptr<ITypeDescriptor>>				tds;
				List<Ptr<IMethodInfo>>					mis;
				List<Ptr<IPropertyInfo>>				pis;
				List<Ptr<IEventInfo>>					eis;
			};

/***********************************************************************
MetaonlyTypeInfo
***********************************************************************/

			class MetaonlyTypeInfo : public Object, public ITypeInfo
			{
				friend struct stream::internal::Serialization<MetaonlyTypeInfo>;
			protected:
				MetaonlyReaderContext*			context = nullptr;
				Decorator						decorator = TypeDescriptor;
				TypeInfoHint					hint = TypeInfoHint::Normal;
				Ptr<MetaonlyTypeInfo>			elementType;
				vint							typeDecriptor = -1;
				List<Ptr<MetaonlyTypeInfo>>		genericArguments;

			public:
				MetaonlyTypeInfo() = default;

				MetaonlyTypeInfo(MetaonlyWriterContext& _context, ITypeInfo* typeInfo)
					: decorator(typeInfo->GetDecorator())
					, hint(typeInfo->GetHint())
					, typeDecriptor(_context.tdIndex[typeInfo->GetTypeDescriptor()])
				{
					if (auto et = typeInfo->GetElementType())
					{
						elementType = new MetaonlyTypeInfo(_context, et);
					}
					for (vint i = 0; i < typeInfo->GetGenericArgumentCount(); i++)
					{
						auto ga = typeInfo->GetGenericArgument(i);
						genericArguments.Add(new MetaonlyTypeInfo(_context, ga));
					}
				}

				void SetContext(MetaonlyReaderContext* _context)
				{
					context = _context;
					if (elementType)
					{
						elementType->SetContext(_context);
					}
					for (vint i = 0; i < genericArguments.Count(); i++)
					{
						genericArguments[i]->SetContext(_context);
					}
				}

				Decorator GetDecorator() override
				{
					return decorator;
				}

				TypeInfoHint GetHint() override
				{
					return hint;
				}

				ITypeInfo* GetElementType() override
				{
					return elementType.Obj();
				}

				ITypeDescriptor* GetTypeDescriptor() override
				{
					return context->tds[typeDecriptor].Obj();
				}

				vint GetGenericArgumentCount() override
				{
					return genericArguments.Count();
				}

				ITypeInfo* GetGenericArgument(vint index) override
				{
					return genericArguments[index].Obj();
				}

				WString GetTypeFriendlyName() override
				{
					switch (decorator)
					{
					case RawPtr: return elementType->GetTypeFriendlyName() + L"*";
					case SharedPtr: return elementType->GetTypeFriendlyName() + L"^";
					case Nullable: return elementType->GetTypeFriendlyName() + L"?";
					case TypeDescriptor: return GetTypeDescriptor()->GetTypeName();
					}
					WString result = elementType->GetTypeFriendlyName() + L"<";
					FOREACH_INDEXER(Ptr<MetaonlyTypeInfo>, type, i, genericArguments)
					{
						if (i > 0) result += L", ";
						result += type->GetTypeFriendlyName();
					}
					result += L">";
					return result;
				}
			};

			ITypeInfo* EnsureSetContext(ITypeInfo* info, MetaonlyReaderContext* context)
			{
				if (auto minfo = dynamic_cast<MetaonlyTypeInfo*>(info))
				{
					minfo->SetContext(context);
				}
				return info;
			}

/***********************************************************************
Metadata
***********************************************************************/

			struct IdRange
			{
				vint								start = -1;
				vint								count = 0;
			};

			struct ParameterInfoMetadata
			{
				WString								name;
				Ptr<MetaonlyTypeInfo>				type;
			};

			struct MethodInfoMetadata
			{
				WString								invokeTemplate;
				WString								closureTemplate;
				WString								name;
				vint								ownerTypeDescriptor = -1;
				vint								ownerProperty = -1;
				List<Ptr<ParameterInfoMetadata>>	parameters;
				Ptr<MetaonlyTypeInfo>				returnType;
				bool								isStatic = false;
			};

			struct PropertyInfoMetadata
			{
				WString								referenceTemplate;
				WString								name;
				vint								ownerTypeDescriptor = -1;
				bool								isReadable = false;
				bool								isWritable = false;
				Ptr<MetaonlyTypeInfo>				returnType;
				vint								getter = -1;
				vint								setter = -1;
				vint								valueChangedEvent = -1;
			};

			struct EventInfoMetadata
			{
				WString								attachTemplate;
				WString								detachTemplate;
				WString								invokeTemplate;
				WString								name;
				vint								ownerTypeDescriptor = -1;
				Ptr<MetaonlyTypeInfo>				handlerType;
				List<vint>							observingProperties;
			};

			struct TypeDescriptorMetadata
			{
				WString								fullName;
				WString								typeName;
				TypeDescriptorFlags					flags = TypeDescriptorFlags::Undefined;
				bool								isAggregatable = false;
				bool								isValueType = false;
				bool								isSerializable = false;
				bool								isEnumType = false;
				bool								isFlagEnum = false;
				List<WString>						enumItems;
				List<vuint64_t>						enumValues;
				List<vint>							baseTypeDescriptors;
				List<vint>							properties;
				List<vint>							events;
				List<vint>							methods;
				List<IdRange>						methodGroups;
				IdRange								constructorGroup;
			};
		}
	}

	namespace stream
	{
		namespace internal
		{

/***********************************************************************
Serialization
***********************************************************************/

			SERIALIZE_ENUM(reflection::description::ITypeInfo::Decorator)
			SERIALIZE_ENUM(reflection::description::TypeInfoHint)
			SERIALIZE_ENUM(reflection::description::TypeDescriptorFlags)

			BEGIN_SERIALIZATION(reflection::description::IdRange)
				SERIALIZE(start)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(reflection::description::MetaonlyTypeInfo)
				SERIALIZE(decorator)
				SERIALIZE(hint)
				SERIALIZE(elementType)
				SERIALIZE(typeDecriptor)
				SERIALIZE(genericArguments)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(reflection::description::ParameterInfoMetadata)
				SERIALIZE(name)
				SERIALIZE(type)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(reflection::description::MethodInfoMetadata)
				SERIALIZE(invokeTemplate)
				SERIALIZE(closureTemplate)
				SERIALIZE(name)
				SERIALIZE(ownerTypeDescriptor)
				SERIALIZE(ownerProperty)
				SERIALIZE(parameters)
				SERIALIZE(returnType)
				SERIALIZE(isStatic)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(reflection::description::PropertyInfoMetadata)
				SERIALIZE(referenceTemplate)
				SERIALIZE(name)
				SERIALIZE(ownerTypeDescriptor)
				SERIALIZE(isReadable)
				SERIALIZE(isWritable)
				SERIALIZE(returnType)
				SERIALIZE(getter)
				SERIALIZE(setter)
				SERIALIZE(valueChangedEvent)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(reflection::description::EventInfoMetadata)
				SERIALIZE(attachTemplate)
				SERIALIZE(detachTemplate)
				SERIALIZE(invokeTemplate)
				SERIALIZE(name)
				SERIALIZE(ownerTypeDescriptor)
				SERIALIZE(handlerType)
				SERIALIZE(observingProperties)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(reflection::description::TypeDescriptorMetadata)
				SERIALIZE(fullName)
				SERIALIZE(typeName)
				SERIALIZE(flags)
				SERIALIZE(isAggregatable)
				SERIALIZE(isValueType)
				SERIALIZE(isSerializable)
				SERIALIZE(isEnumType)
				SERIALIZE(isFlagEnum)
				SERIALIZE(enumItems)
				SERIALIZE(enumValues)
				SERIALIZE(baseTypeDescriptors)
				SERIALIZE(properties)
				SERIALIZE(events)
				SERIALIZE(methods)
				SERIALIZE(methodGroups)
				SERIALIZE(constructorGroup)
			END_SERIALIZATION
		}
	}

	namespace reflection
	{
		namespace description
		{

			using Reader = stream::internal::Reader<Ptr<MetaonlyReaderContext>>;
			using Writer = stream::internal::Writer<Ptr<MetaonlyWriterContext>>;

/***********************************************************************
IMethodInfo
***********************************************************************/

			class MetaonlyParameterInfo : public Object, public IParameterInfo
			{
			protected:
				MetaonlyReaderContext*			context = nullptr;
				Ptr<ParameterInfoMetadata>		metadata;
				vint							ownerTypeDescriptor = -1;
				IMethodInfo*					ownerMethod = nullptr;

			public:
				MetaonlyParameterInfo(MetaonlyReaderContext* _context, Ptr<ParameterInfoMetadata> _metadata, vint _ownerTypeDescriptor, IMethodInfo* _ownerMethod)
					: context(_context)
					, metadata(_metadata)
					, ownerTypeDescriptor(_ownerTypeDescriptor)
					, ownerMethod(_ownerMethod)
				{
				}

				ITypeDescriptor* GetOwnerTypeDescriptor() override
				{
					return context->tds[ownerTypeDescriptor].Obj();
				}

				const WString& GetName() override
				{
					return metadata->name;
				}

				ITypeInfo* GetType() override
				{
					return EnsureSetContext(metadata->type.Obj(), context);
				}

				IMethodInfo* GetOwnerMethod() override
				{
					return ownerMethod;
				}
			};

			class MetaonlyMethodInfo : public Object, public IMethodInfo, protected IMethodInfo::ICpp
			{
				friend class MetaonlyMethodGroupInfo;
			protected:
				MetaonlyReaderContext*			context = nullptr;
				Ptr<MethodInfoMetadata>			metadata;
				IMethodGroupInfo*				methodGroup = nullptr;
				List<Ptr<IParameterInfo>>		parameters;

			public:
				MetaonlyMethodInfo(MetaonlyReaderContext* _context, Ptr<MethodInfoMetadata> _metadata)
					: context(_context)
					, metadata(_metadata)
				{
					for (vint i = 0; i < metadata->parameters.Count(); i++)
					{
						parameters.Add(new MetaonlyParameterInfo(context, metadata->parameters[i], metadata->ownerTypeDescriptor, this));
					}
				}

				// ICpp

				const WString& GetInvokeTemplate() override
				{
					return metadata->invokeTemplate;
				}

				const WString& GetClosureTemplate() override
				{
					return metadata->closureTemplate;
				}

				// IMemberInfo

				ITypeDescriptor* GetOwnerTypeDescriptor() override
				{
					return context->tds[metadata->ownerTypeDescriptor].Obj();
				}

				const WString& GetName() override
				{
					return metadata->name;
				}

				// IMethodInfo

				ICpp* GetCpp() override
				{
					if (metadata->invokeTemplate.Length() + metadata->closureTemplate.Length() > 0)
					{
						return this;
					}
					return nullptr;
				}

				IMethodGroupInfo* GetOwnerMethodGroup() override
				{
					return methodGroup;
				}

				IPropertyInfo* GetOwnerProperty() override
				{
					return metadata->ownerProperty == -1 ? nullptr : context->pis[metadata->ownerProperty].Obj();
				}

				vint GetParameterCount() override
				{
					return parameters.Count();
				}

				IParameterInfo* GetParameter(vint index) override
				{
					return parameters[index].Obj();
				}

				ITypeInfo* GetReturn() override
				{
					return EnsureSetContext(metadata->returnType.Obj(), context);
				}

				bool IsStatic() override
				{
					return metadata->isStatic;
				}

				void CheckArguments(collections::Array<Value>& arguments) override
				{
					CHECK_FAIL(L"Not Supported!");
				}

				Value Invoke(const Value& thisObject, collections::Array<Value>& arguments) override
				{
					CHECK_FAIL(L"Not Supported!");
				}

				Value CreateFunctionProxy(const Value& thisObject) override
				{
					CHECK_FAIL(L"Not Supported!");
				}
			};

			class MetaonlyMethodGroupInfo : public Object, public IMethodGroupInfo
			{
			protected:
				MetaonlyReaderContext*			context = nullptr;
				Ptr<TypeDescriptorMetadata>		metadata;
				IdRange							idRange;
			public:
				MetaonlyMethodGroupInfo(MetaonlyReaderContext* _context, Ptr<TypeDescriptorMetadata> _metadata, IdRange _idRange)
					: context(_context)
					, metadata(_metadata)
					, idRange(_idRange)
				{
				}

				// IMemberInfo

				ITypeDescriptor* GetOwnerTypeDescriptor() override
				{
					return GetMethod(0)->GetOwnerTypeDescriptor();
				}

				const WString& GetName() override
				{
					return GetMethod(0)->GetName();
				}

				// IMethodGroupInfo

				vint GetMethodCount() override
				{
					return idRange.count;
				}

				IMethodInfo* GetMethod(vint index) override
				{
					CHECK_ERROR(0 <= index && index < idRange.count, L"IMethodGroupInfo::GetMethod(vint)#Index out of range.");
					auto info = dynamic_cast<MetaonlyMethodInfo*>(context->mis[metadata->methods[idRange.start + index]].Obj());
					if (info->methodGroup == nullptr)
					{
						info->methodGroup = this;
					}
					return info;
				}
			};

/***********************************************************************
IPropertyInfo
***********************************************************************/

			class MetaonlyPropertyInfo : public Object, public IPropertyInfo, protected IPropertyInfo::ICpp
			{
			protected:
				MetaonlyReaderContext*			context = nullptr;
				Ptr<PropertyInfoMetadata>		metadata;

			public:
				MetaonlyPropertyInfo(MetaonlyReaderContext* _context, Ptr<PropertyInfoMetadata> _metadata)
					: context(_context)
					, metadata(_metadata)
				{
				}

				// ICpp

				const WString& GetReferenceTemplate() override
				{
					return metadata->referenceTemplate;
				}

				// IMemberInfo

				ITypeDescriptor* GetOwnerTypeDescriptor() override
				{
					return context->tds[metadata->ownerTypeDescriptor].Obj();
				}

				const WString& GetName() override
				{
					return metadata->name;
				}

				// IPropertyInfo

				ICpp* GetCpp() override
				{
					if (metadata->referenceTemplate.Length() > 0)
					{
						return this;
					}
					return nullptr;
				}

				bool IsReadable() override
				{
					return metadata->isReadable;
				}

				bool IsWritable() override
				{
					return metadata->isWritable;
				}

				ITypeInfo* GetReturn() override
				{
					return EnsureSetContext(metadata->returnType.Obj(), context);
				}

				IMethodInfo* GetGetter() override
				{
					return metadata->getter == -1 ? nullptr : context->mis[metadata->getter].Obj();
				}

				IMethodInfo* GetSetter() override
				{
					return metadata->setter == -1 ? nullptr : context->mis[metadata->setter].Obj();
				}

				IEventInfo* GetValueChangedEvent() override
				{
					return metadata->valueChangedEvent == -1 ? nullptr : context->eis[metadata->valueChangedEvent].Obj();
				}

				Value GetValue(const Value& thisObject) override
				{
					CHECK_FAIL(L"Not Supported!");
				}

				void SetValue(Value& thisObject, const Value& newValue) override
				{
					CHECK_FAIL(L"Not Supported!");
				}
			};

/***********************************************************************
IEventInfo
***********************************************************************/

			class MetaonlyEventInfo : public Object, public IEventInfo, protected IEventInfo::ICpp
			{
			protected:
				MetaonlyReaderContext*			context = nullptr;
				Ptr<EventInfoMetadata>			metadata;

			public:
				MetaonlyEventInfo(MetaonlyReaderContext* _context, Ptr<EventInfoMetadata> _metadata)
					: context(_context)
					, metadata(_metadata)
				{
				}

				// ICpp

				const WString& GetAttachTemplate() override
				{
					return metadata->attachTemplate;
				}

				const WString& GetDetachTemplate() override
				{
					return metadata->detachTemplate;
				}

				const WString& GetInvokeTemplate() override
				{
					return metadata->invokeTemplate;
				}

				// IMemberInfo

				ITypeDescriptor* GetOwnerTypeDescriptor() override
				{
					return context->tds[metadata->ownerTypeDescriptor].Obj();
				}

				const WString& GetName() override
				{
					return metadata->name;
				}

				// IEventInfo

				ICpp* GetCpp() override
				{
					if (metadata->attachTemplate.Length() + metadata->detachTemplate.Length() + metadata->invokeTemplate.Length() > 0)
					{
						return this;
					}
					return nullptr;
				}

				ITypeInfo* GetHandlerType() override
				{
					return EnsureSetContext(metadata->handlerType.Obj(), context);
				}

				vint GetObservingPropertyCount() override
				{
					return metadata->observingProperties.Count();
				}

				IPropertyInfo* GetObservingProperty(vint index) override
				{
					return context->pis[metadata->observingProperties[index]].Obj();
				}

				Ptr<IEventHandler> Attach(const Value& thisObject, Ptr<IValueFunctionProxy> handler) override
				{
					CHECK_FAIL(L"Not Supported!");
				}

				bool Detach(const Value& thisObject, Ptr<IEventHandler> handler) override
				{
					CHECK_FAIL(L"Not Supported!");
				}

				void Invoke(const Value& thisObject, Ptr<IValueList> arguments) override
				{
					CHECK_FAIL(L"Not Supported!");
				}
			};

/***********************************************************************
ITypeDescriptor
***********************************************************************/

			class MetaonlyTypeDescriptor
				: public Object
				, public ITypeDescriptor
				, protected ITypeDescriptor::ICpp
				, protected IValueType
				, protected IEnumType
			{
			protected:
				MetaonlyReaderContext*			context = nullptr;
				Ptr<TypeDescriptorMetadata>		metadata;
				ISerializableType*				serializableType = nullptr;
				List<Ptr<IMethodGroupInfo>>		methodGroups;
				Ptr<IMethodGroupInfo>			constructorGroup;

			public:
				MetaonlyTypeDescriptor(MetaonlyReaderContext* _context, Ptr<TypeDescriptorMetadata> _metadata)
					: context(_context)
					, metadata(_metadata)
				{
					if (metadata->isSerializable)
					{
						serializableType = context->serializableTypes[metadata->typeName].Obj();
					}

					for (vint i = 0; i < metadata->methodGroups.Count(); i++)
					{
						methodGroups.Add(new MetaonlyMethodGroupInfo(context, metadata, metadata->methodGroups[i]));
					}
					if (metadata->constructorGroup.start != -1)
					{
						constructorGroup = new MetaonlyMethodGroupInfo(context, metadata, metadata->constructorGroup);
					}
				}

				// ICpp

				const WString& GetFullName() override
				{
					return metadata->fullName;
				}

				// IValueType

				Value CreateDefault() override
				{
					CHECK_FAIL(L"Not Supported!");
				}

				IBoxedValue::CompareResult Compare(const Value& a, const Value& b) override
				{
					CHECK_FAIL(L"Not Supported!");
				}

				// IEnumType

				bool IsFlagEnum() override
				{
					return metadata->isFlagEnum;
				}

				vint GetItemCount() override
				{
					return metadata->enumItems.Count();
				}

				WString GetItemName(vint index) override
				{
					return metadata->enumItems[index];
				}

				vuint64_t GetItemValue(vint index) override
				{
					return metadata->enumValues[index];
				}

				vint IndexOfItem(WString name) override
				{
					return metadata->enumItems.IndexOf(name);
				}

				Value ToEnum(vuint64_t value) override
				{
					CHECK_FAIL(L"Not Supported!");
				}

				vuint64_t FromEnum(const Value& value) override
				{
					CHECK_FAIL(L"Not Supported!");
				}

				// ITypeDescriptor

				ICpp* GetCpp() override
				{
					if (metadata->fullName.Length() > 0)
					{
						return this;
					}
					return nullptr;
				}

				TypeDescriptorFlags GetTypeDescriptorFlags() override
				{
					return metadata->flags;
				}

				bool IsAggregatable() override
				{
					return metadata->isAggregatable;
				}

				const WString& GetTypeName() override
				{
					return metadata->typeName;
				}

				IValueType* GetValueType() override
				{
					return metadata->isValueType ? this : nullptr;
				}

				IEnumType* GetEnumType() override
				{
					return metadata->isEnumType ? this : nullptr;
				}

				ISerializableType* GetSerializableType() override
				{
					return serializableType;
				}

				vint GetBaseTypeDescriptorCount() override
				{
					return metadata->baseTypeDescriptors.Count();
				}

				ITypeDescriptor* GetBaseTypeDescriptor(vint index) override
				{
					return context->tds[metadata->baseTypeDescriptors[index]].Obj();
				}

				bool CanConvertTo(ITypeDescriptor* targetType) override
				{
					if (this == targetType) return true;
					vint count = GetBaseTypeDescriptorCount();
					for (vint i = 0; i < count; i++)
					{
						if (GetBaseTypeDescriptor(i)->CanConvertTo(targetType)) return true;
					}
					return false;
				}

				vint GetPropertyCount() override
				{
					return metadata->properties.Count();
				}

				IPropertyInfo* GetProperty(vint index) override
				{
					return context->pis[metadata->properties[index]].Obj();
				}

				bool IsPropertyExists(const WString& name, bool inheritable) override
				{
					return GetPropertyByName(name, inheritable);
				}

				IPropertyInfo* GetPropertyByName(const WString& name, bool inheritable) override
				{
					for (vint i = 0; i < metadata->properties.Count(); i++)
					{
						auto info = GetProperty(i);
						if (info->GetName() == name)
						{
							return info;
						}
					}
					if (inheritable)
					{
						for (vint i = 0; i < metadata->baseTypeDescriptors.Count(); i++)
						{
							if (auto info = GetBaseTypeDescriptor(i)->GetPropertyByName(name, true))
							{
								return info;
							}
						}
					}
					return nullptr;
				}

				vint GetEventCount() override
				{
					return metadata->events.Count();
				}

				IEventInfo* GetEvent(vint index) override
				{
					return context->eis[metadata->events[index]].Obj();
				}

				bool IsEventExists(const WString& name, bool inheritable) override
				{
					return GetEventByName(name, inheritable);
				}

				IEventInfo* GetEventByName(const WString& name, bool inheritable) override
				{
					for (vint i = 0; i < metadata->events.Count(); i++)
					{
						auto info = GetEvent(i);
						if (info->GetName() == name)
						{
							return info;
						}
					}
					if (inheritable)
					{
						for (vint i = 0; i < metadata->baseTypeDescriptors.Count(); i++)
						{
							if (auto info = GetBaseTypeDescriptor(i)->GetEventByName(name, true))
							{
								return info;
							}
						}
					}
					return nullptr;
				}

				vint GetMethodGroupCount() override
				{
					return methodGroups.Count();
				}

				IMethodGroupInfo* GetMethodGroup(vint index) override
				{
					return methodGroups[index].Obj();
				}

				bool IsMethodGroupExists(const WString& name, bool inheritable) override
				{
					return GetMethodGroupByName(name, inheritable);
				}

				IMethodGroupInfo* GetMethodGroupByName(const WString& name, bool inheritable) override
				{
					for (vint i = 0; i < methodGroups.Count(); i++)
					{
						auto info = methodGroups[i].Obj();
						if (info->GetName() == name)
						{
							return info;
						}
					}
					if (inheritable)
					{
						for (vint i = 0; i < metadata->baseTypeDescriptors.Count(); i++)
						{
							if (auto info = GetBaseTypeDescriptor(i)->GetMethodGroupByName(name, true))
							{
								return info;
							}
						}
					}
					return nullptr;
				}

				IMethodGroupInfo* GetConstructorGroup() override
				{
					return constructorGroup.Obj();
				}
			};

/***********************************************************************
GenerateMetaonlyTypes
***********************************************************************/

			void GenerateMetaonlyTypeDescriptor(Writer& writer, ITypeDescriptor* td)
			{
				auto metadata = MakePtr<TypeDescriptorMetadata>();
				if (auto cpp = td->GetCpp())
				{
					metadata->fullName = cpp->GetFullName();
				}
				metadata->typeName = td->GetTypeName();
				metadata->flags = td->GetTypeDescriptorFlags();
				metadata->isAggregatable = td->IsAggregatable();
				metadata->isValueType = td->GetValueType();
				metadata->isSerializable = td->GetSerializableType();
				if (auto enumType = td->GetEnumType())
				{
					metadata->isEnumType = true;
					metadata->isFlagEnum = enumType->IsFlagEnum();
					for (vint i = 0; i < enumType->GetItemCount(); i++)
					{
						metadata->enumItems.Add(enumType->GetItemName(i));
						metadata->enumValues.Add(enumType->GetItemValue(i));
					}
				}

				for (vint i = 0; i < td->GetBaseTypeDescriptorCount(); i++)
				{
					metadata->baseTypeDescriptors.Add(writer.context->tdIndex[td->GetBaseTypeDescriptor(i)]);
				}

				for (vint i = 0; i < td->GetPropertyCount(); i++)
				{
					metadata->properties.Add(writer.context->piIndex[td->GetProperty(i)]);
				}

				for (vint i = 0; i < td->GetEventCount(); i++)
				{
					metadata->events.Add(writer.context->eiIndex[td->GetEvent(i)]);
				}

				for (vint i = 0; i < td->GetMethodGroupCount(); i++)
				{
					auto mg = td->GetMethodGroup(i);
					IdRange ir;
					ir.start = metadata->methods.Count();
					for (vint j = 0; j < mg->GetMethodCount(); j++)
					{
						metadata->methods.Add(writer.context->miIndex[mg->GetMethod(j)]);
					}
					ir.count = metadata->methods.Count() - ir.start;
					metadata->methodGroups.Add(ir);
				}

				if (auto cg = td->GetConstructorGroup())
				{
					metadata->constructorGroup.start = metadata->methods.Count();
					for (vint j = 0; j < cg->GetMethodCount(); j++)
					{
						metadata->methods.Add(writer.context->miIndex[cg->GetMethod(j)]);
					}
					metadata->constructorGroup.count = metadata->methods.Count() - metadata->constructorGroup.start;
				}

				writer << metadata;
			}

			void GenerateMetaonlyMethodInfo(Writer& writer, IMethodInfo* mi)
			{
				auto metadata = MakePtr<MethodInfoMetadata>();
				if (auto cpp = mi->GetCpp())
				{
					metadata->invokeTemplate = cpp->GetInvokeTemplate();
					metadata->closureTemplate = cpp->GetClosureTemplate();
				}
				metadata->name = mi->GetName();
				metadata->ownerTypeDescriptor = writer.context->tdIndex[mi->GetOwnerTypeDescriptor()];
				if (auto pi = mi->GetOwnerProperty())
				{
					metadata->ownerProperty = writer.context->piIndex[pi];
				}
				for (vint i = 0; i < mi->GetParameterCount(); i++)
				{
					auto pi = mi->GetParameter(i);
					auto piMetadata = MakePtr<ParameterInfoMetadata>();
					piMetadata->name = pi->GetName();
					piMetadata->type = new MetaonlyTypeInfo(*writer.context.Obj(), pi->GetType());
					metadata->parameters.Add(piMetadata);
				}
				metadata->returnType = new MetaonlyTypeInfo(*writer.context.Obj(), mi->GetReturn());
				metadata->isStatic = mi->IsStatic();
				writer << metadata;
			}

			void GenerateMetaonlyPropertyInfo(Writer& writer, IPropertyInfo* pi)
			{
				auto metadata = MakePtr<PropertyInfoMetadata>();;
				if (auto cpp = pi->GetCpp())
				{
					metadata->referenceTemplate = cpp->GetReferenceTemplate();
				}
				metadata->name = pi->GetName();
				metadata->ownerTypeDescriptor = writer.context->tdIndex[pi->GetOwnerTypeDescriptor()];
				metadata->isReadable = pi->IsReadable();
				metadata->isWritable = pi->IsWritable();
				metadata->returnType = new MetaonlyTypeInfo(*writer.context.Obj(), pi->GetReturn());
				if (auto mi = pi->GetGetter())
				{
					metadata->getter = writer.context->miIndex[mi];
				}
				if (auto mi = pi->GetSetter())
				{
					metadata->setter = writer.context->miIndex[mi];
				}
				if (auto ei = pi->GetValueChangedEvent())
				{
					metadata->valueChangedEvent = writer.context->eiIndex[ei];
				}
				writer << metadata;
			}

			void GenerateMetaonlyEventInfo(Writer& writer, IEventInfo* ei)
			{
				auto metadata = MakePtr<EventInfoMetadata>();;
				if (auto cpp = ei->GetCpp())
				{
					metadata->attachTemplate = cpp->GetAttachTemplate();
					metadata->detachTemplate = cpp->GetDetachTemplate();
					metadata->invokeTemplate = cpp->GetInvokeTemplate();
				}
				metadata->name = ei->GetName();
				metadata->ownerTypeDescriptor = writer.context->tdIndex[ei->GetOwnerTypeDescriptor()];
				metadata->handlerType = new MetaonlyTypeInfo(*writer.context.Obj(), ei->GetHandlerType());
				for (vint i = 0; i < ei->GetObservingPropertyCount(); i++)
				{
					metadata->observingProperties.Add(writer.context->piIndex[ei->GetObservingProperty(i)]);
				}
				writer << metadata;
			}

			void GenerateMetaonlyTypes(stream::IStream& outputStream)
			{
				Writer writer(outputStream);
				writer.context = MakePtr<MetaonlyWriterContext>();

				Dictionary<WString, ITypeDescriptor*> tds;
				List<IMethodInfo*> mis;
				List<IPropertyInfo*> pis;
				List<IEventInfo*> eis;

				{
					auto tm = GetGlobalTypeManager();
					vint count = tm->GetTypeDescriptorCount();

					for (vint i = 0; i < count; i++)
					{
						auto td = tm->GetTypeDescriptor(i);
						tds.Add(td->GetTypeName(), td);
					}
				}
				{
					vint count = tds.Count();
					for (vint i = 0; i < count; i++)
					{
						auto td = tds.Values()[i];
						writer.context->tdIndex.Add(td, writer.context->tdIndex.Count());

						vint mgCount = td->GetMethodGroupCount();
						for (vint j = 0; j < mgCount; j++)
						{
							auto mg = td->GetMethodGroup(j);
							vint miCount = mg->GetMethodCount();
							for (vint k = 0; k < miCount; k++)
							{
								auto mi = mg->GetMethod(k);
								writer.context->miIndex.Add(mi, mis.Count());
								mis.Add(mi);
							}
						}

						if (auto cg = td->GetConstructorGroup())
						{
							vint miCount = cg->GetMethodCount();
							for (vint k = 0; k < miCount; k++)
							{
								auto mi = cg->GetMethod(k);
								writer.context->miIndex.Add(mi, mis.Count());
								mis.Add(mi);
							}
						}

						vint piCount = td->GetPropertyCount();
						for (vint j = 0; j < piCount; j++)
						{
							auto pi = td->GetProperty(j);
							writer.context->piIndex.Add(pi, pis.Count());
							pis.Add(pi);
						}

						vint eiCount = td->GetEventCount();
						for (vint j = 0; j < eiCount; j++)
						{
							auto ei = td->GetEvent(j);
							writer.context->eiIndex.Add(ei, eis.Count());
							eis.Add(ei);
						}
					}
				}
				{
					vint tdCount = tds.Count();
					vint miCount = mis.Count();
					vint piCount = pis.Count();
					vint eiCount = eis.Count();
					writer << tdCount << miCount << piCount << eiCount;

					for (vint i = 0; i < tdCount; i++)
					{
						GenerateMetaonlyTypeDescriptor(writer, tds.Values()[i]);
					}
					for (vint i = 0; i < miCount; i++)
					{
						GenerateMetaonlyMethodInfo(writer, mis[i]);
					}
					for (vint i = 0; i < piCount; i++)
					{
						GenerateMetaonlyPropertyInfo(writer, pis[i]);
					}
					for (vint i = 0; i < eiCount; i++)
					{
						GenerateMetaonlyEventInfo(writer, eis[i]);
					}
				}
			}

/***********************************************************************
LoadMetaonlyTypes
***********************************************************************/

			class MetaonlyTypeLoader : public Object, public ITypeLoader
			{
			public:
				Ptr<MetaonlyReaderContext>				context;

				void Load(ITypeManager* manager) override
				{
					for (vint i = 0; i < context->tds.Count(); i++)
					{
						auto td = context->tds[i];
						manager->SetTypeDescriptor(td->GetTypeName(), td);
					}
				}

				void Unload(ITypeManager* manager) override
				{
				}
			};

			Ptr<ITypeLoader> LoadMetaonlyTypes(stream::IStream& inputStream, const collections::Dictionary<WString, Ptr<ISerializableType>>& serializableTypes)
			{
				auto context = MakePtr<MetaonlyReaderContext>();
				CopyFrom(context->serializableTypes, serializableTypes);
				auto loader = MakePtr<MetaonlyTypeLoader>();
				loader->context = context;
				Reader reader(inputStream);
				reader.context = context;

				{
					vint tdCount = 0;
					vint miCount = 0;
					vint piCount = 0;
					vint eiCount = 0;
					reader << tdCount << miCount << piCount << eiCount;

					for (vint i = 0; i < tdCount; i++)
					{
						Ptr<TypeDescriptorMetadata> metadata;
						reader << metadata;
						context->tds.Add(new MetaonlyTypeDescriptor(context.Obj(), metadata));
					}
					for (vint i = 0; i < miCount; i++)
					{
						Ptr<MethodInfoMetadata> metadata;
						reader << metadata;
						context->mis.Add(new MetaonlyMethodInfo(context.Obj(), metadata));
					}
					for (vint i = 0; i < piCount; i++)
					{
						Ptr<PropertyInfoMetadata> metadata;
						reader << metadata;
						context->pis.Add(new MetaonlyPropertyInfo(context.Obj(), metadata));
					}
					for (vint i = 0; i < eiCount; i++)
					{
						Ptr<EventInfoMetadata> metadata;
						reader << metadata;
						context->eis.Add(new MetaonlyEventInfo(context.Obj(), metadata));
					}
				}

				return loader;
			}
		}
	}
}

#endif