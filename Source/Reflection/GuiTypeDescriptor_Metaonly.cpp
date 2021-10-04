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
				List<ITypeDescriptor*>					tds;
				List<IMethodInfo*>						mis;
				List<IPropertyInfo*>					pis;
				List<IEventInfo*>						eis;
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
					return context->tds[typeDecriptor];
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

/***********************************************************************
Metadata
***********************************************************************/

			struct IdRange
			{
				vint								start = -1;
				vint								count = 0;
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
				List<vint>							baseTypeDescriptors;
				List<vint>							properties;
				List<vint>							events;
				List<vint>							methods;
				List<IdRange>						methodGroups;
				IdRange								constructorGroup;
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
				vint								ownerProperty = -1;
				Ptr<MetaonlyTypeInfo>				returnType;
				List<ParameterInfoMetadata>			parameters;
				bool								isStatic = false;
			};

			struct PropertyInfoMetadata
			{
				WString								referenceTemplate;
				WString								name;
				bool								readable = false;
				bool								writable = false;
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
				Ptr<MetaonlyTypeInfo>				handlerType;
				List<vint>							observingProperties;
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
				SERIALIZE(baseTypeDescriptors)
				SERIALIZE(properties)
				SERIALIZE(events)
				SERIALIZE(methods)
				SERIALIZE(methodGroups)
				SERIALIZE(constructorGroup)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(reflection::description::ParameterInfoMetadata)
				SERIALIZE(name)
				SERIALIZE(type)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(reflection::description::MethodInfoMetadata)
				SERIALIZE(invokeTemplate)
				SERIALIZE(closureTemplate)
				SERIALIZE(name)
				SERIALIZE(ownerProperty)
				SERIALIZE(returnType)
				SERIALIZE(parameters)
				SERIALIZE(isStatic)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(reflection::description::PropertyInfoMetadata)
				SERIALIZE(referenceTemplate)
				SERIALIZE(name)
				SERIALIZE(readable)
				SERIALIZE(writable)
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
				SERIALIZE(handlerType)
				SERIALIZE(observingProperties)
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
ITypeDescriptor
***********************************************************************/

			class MetaonlyTypeDescriptor : public Object, public ITypeDescriptor
			{
			protected:
				MetaonlyReaderContext*			context;
				Ptr<TypeDescriptorMetadata>		metadata;

			public:
				MetaonlyTypeDescriptor(MetaonlyReaderContext* _context, Ptr<TypeDescriptorMetadata> _metadata)
					: context(_context)
					, metadata(_metadata)
				{
				}

				ICpp* GetCpp() override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				TypeDescriptorFlags GetTypeDescriptorFlags() override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				bool IsAggregatable() override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				const WString& GetTypeName() override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				IValueType* GetValueType() override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				IEnumType* GetEnumType() override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				ISerializableType* GetSerializableType() override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				vint GetBaseTypeDescriptorCount() override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				ITypeDescriptor* GetBaseTypeDescriptor(vint index) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				bool CanConvertTo(ITypeDescriptor* targetType) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				vint GetPropertyCount() override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				IPropertyInfo* GetProperty(vint index) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				bool IsPropertyExists(const WString& name, bool inheritable) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				IPropertyInfo* GetPropertyByName(const WString& name, bool inheritable) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				vint GetEventCount() override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				IEventInfo* GetEvent(vint index) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				bool IsEventExists(const WString& name, bool inheritable) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				IEventInfo* GetEventByName(const WString& name, bool inheritable) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				vint GetMethodGroupCount() override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				IMethodGroupInfo* GetMethodGroup(vint index) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				bool IsMethodGroupExists(const WString& name, bool inheritable) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				IMethodGroupInfo* GetMethodGroupByName(const WString& name, bool inheritable) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				IMethodGroupInfo* GetConstructorGroup() override
				{
					CHECK_FAIL(L"Not Implemented!");
				}
			};

/***********************************************************************
GenerateMetaonlyTypes
***********************************************************************/

			void GenerateMetaonlyTypeDescriptor(Writer& writer, ITypeDescriptor* td)
			{
				TypeDescriptorMetadata metadata;
				if (auto cpp = td->GetCpp())
				{
					metadata.fullName = cpp->GetFullName();
				}
				metadata.typeName = td->GetTypeName();
				metadata.flags = td->GetTypeDescriptorFlags();
				metadata.isAggregatable = td->IsAggregatable();
				metadata.isValueType = td->GetValueType();
				metadata.isSerializable = td->GetSerializableType();
				if (auto enumType = td->GetEnumType())
				{
					metadata.isEnumType = true;
					metadata.isFlagEnum = enumType->IsFlagEnum();
					for (vint i = 0; i < enumType->GetItemCount(); i++)
					{
						metadata.enumItems.Add(enumType->GetItemName(i));
					}
				}

				for (vint i = 0; i < td->GetBaseTypeDescriptorCount(); i++)
				{
					metadata.baseTypeDescriptors.Add(writer.context->tdIndex[td->GetBaseTypeDescriptor(i)]);
				}

				for (vint i = 0; i < td->GetPropertyCount(); i++)
				{
					metadata.properties.Add(writer.context->piIndex[td->GetProperty(i)]);
				}

				for (vint i = 0; i < td->GetEventCount(); i++)
				{
					metadata.events.Add(writer.context->eiIndex[td->GetEvent(i)]);
				}

				for (vint i = 0; i < td->GetMethodGroupCount(); i++)
				{
					auto mg = td->GetMethodGroup(i);
					IdRange ir;
					ir.start = metadata.methods.Count();
					for (vint j = 0; j > mg->GetMethodCount(); j++)
					{
						metadata.methods.Add(writer.context->miIndex[mg->GetMethod(j)]);
					}
					ir.count = metadata.methods.Count() - ir.start;
					metadata.methodGroups.Add(ir);
				}

				if (auto cg = td->GetConstructorGroup())
				{
					metadata.constructorGroup.start = metadata.methods.Count();
					for (vint j = 0; j > cg->GetMethodCount(); j++)
					{
						metadata.methods.Add(writer.context->miIndex[cg->GetMethod(j)]);
					}
					metadata.constructorGroup.count = metadata.methods.Count() - metadata.constructorGroup.start;
				}

				writer << metadata;
			}

			void GenerateMetaonlyMethodInfo(Writer& writer, IMethodInfo* mi)
			{
				MethodInfoMetadata metadata;
				writer << metadata;
			}

			void GenerateMetaonlyPropertyInfo(Writer& writer, IPropertyInfo* pi)
			{
				PropertyInfoMetadata metadata;
				writer << metadata;
			}

			void GenerateMetaonlyEventInfo(Writer& writer, IEventInfo* ei)
			{
				EventInfoMetadata metadata;
				if (auto cpp = ei->GetCpp())
				{
					metadata.attachTemplate = cpp->GetAttachTemplate();
					metadata.detachTemplate = cpp->GetDetachTemplate();
					metadata.invokeTemplate = cpp->GetInvokeTemplate();
				}
				metadata.name = ei->GetName();
				metadata.handlerType = new MetaonlyTypeInfo(*writer.context.Obj(), ei->GetHandlerType());
				for (vint i = 0; i < ei->GetObservingPropertyCount(); i++)
				{
					metadata.observingProperties.Add(writer.context->piIndex[ei->GetObservingProperty(i)]);
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
					vint count = tds.Count();
					writer << count;
					for (vint i = 0; i < count; i++)
					{
						auto name = tds.Keys()[i];
						writer << name;
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

			void LoadMetaonlyTypes(stream::IStream& inputStream)
			{
				Reader reader(inputStream);
			}
		}
	}
}

#endif