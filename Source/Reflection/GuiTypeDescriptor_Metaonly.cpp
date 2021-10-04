/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "GuiTypeDescriptorReflection.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			using namespace collections;

			struct IdRange
			{
				vint			start = -1;
				vint			count = 0;
			};

			struct TypeDescriptorMetadata
			{
				WString						fullName;
				WString						typeName;
				TypeDescriptorFlags			flags;
				bool						isAggregatable;
				bool						isValueType;
				bool						isSerializable;
				bool						isEnumType;
				bool						isFlagEnum;
				List<WString>				enumItems;
				List<vint>					baseTypeDescriptors;
				List<vint>					properties;
				List<vint>					events;
				List<vint>					methods;
				List<IdRange>				methodGroups;
				IdRange						constructorGroup;
			};
		}
	}

	namespace stream
	{
		namespace internal
		{
			SERIALIZE_ENUM(reflection::description::ITypeInfo::Decorator)
			SERIALIZE_ENUM(reflection::description::TypeInfoHint)
			SERIALIZE_ENUM(reflection::description::TypeDescriptorFlags)

			BEGIN_SERIALIZATION(reflection::description::IdRange)
				SERIALIZE(start)
				SERIALIZE(count)
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
		}
	}

	namespace reflection
	{
		namespace description
		{

#ifndef VCZH_DEBUG_NO_REFLECTION

			struct MetaonlyWriterContext
			{
				Dictionary<ITypeDescriptor*, vint>		tdIndex;
				Dictionary<IMethodInfo*, vint>			miIndex;
				Dictionary<IPropertyInfo*, vint>		piIndex;
				Dictionary<IEventInfo*, vint>			eiIndex;
			};

			struct MetaonlyReaderContext
			{

			};

			using Reader = stream::internal::Reader<Ptr<MetaonlyReaderContext>>;
			using Writer = stream::internal::Writer<Ptr<MetaonlyWriterContext>>;

/***********************************************************************
ITypeInfo
***********************************************************************/

			class MetaonlyTypeInfo : public Object, public ITypeInfo
			{
			protected:
				Decorator						decorator = TypeDescriptor;
				TypeInfoHint					hint = TypeInfoHint::Normal;
				ITypeInfo*						elementType = nullptr;
				ITypeDescriptor*				typeDecriptor=nullptr;
				List<Ptr<MetaonlyTypeInfo>>		genericArguments;
				WString							typeFriendlyName;
			};

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
				else
				{
					metadata.isEnumType = false;
					metadata.isFlagEnum = false;
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

			void GenerateMetaonlyMethodInfo(Writer& writer, IMethodInfo* td)
			{
			}

			void GenerateMetaonlyPropertyInfo(Writer& writer, IPropertyInfo* td)
			{
			}

			void GenerateMetaonlyEventInfo(Writer& writer, IEventInfo* td)
			{
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

#endif
		}
	}
}
