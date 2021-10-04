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
