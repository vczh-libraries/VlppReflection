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

			void GenerateMetaonlyTypes(stream::IStream& outputStream)
			{
				Writer writer(outputStream);
				writer.context = MakePtr<MetaonlyWriterContext>();

				Dictionary<WString, ITypeDescriptor*> tds;
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
								writer.context->miIndex.Add(mg->GetMethod(k), writer.context->miIndex.Count());
							}
						}

						if (auto cg = td->GetConstructorGroup())
						{
							vint miCount = cg->GetMethodCount();
							for (vint k = 0; k < miCount; k++)
							{
								writer.context->miIndex.Add(cg->GetMethod(k), writer.context->miIndex.Count());
							}
						}

						vint piCount = td->GetPropertyCount();
						for (vint j = 0; j < piCount; j++)
						{
							writer.context->piIndex.Add(td->GetProperty(j), writer.context->piIndex.Count());
						}

						vint eiCount = td->GetEventCount();
						for (vint j = 0; j < eiCount; j++)
						{
							writer.context->eiIndex.Add(td->GetEvent(j), writer.context->eiIndex.Count());
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
