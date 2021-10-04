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

			using ContextFreeReader = stream::internal::ContextFreeReader;
			using ContextFreeWriter = stream::internal::ContextFreeWriter;

#ifndef VCZH_DEBUG_NO_REFLECTION

/***********************************************************************
GenerateMetaonlyTypes
***********************************************************************/

			void GenerateMetaonlyTypes(stream::IStream& outputStream)
			{
				ContextFreeWriter writer(outputStream);
				Dictionary<WString, ITypeDescriptor*> tds;
				{
					auto tm = GetGlobalTypeManager();
					vint count = tm->GetTypeDescriptorCount();
					writer << count;

					for (vint i = 0; i < count; i++)
					{
						auto td = tm->GetTypeDescriptor(i);
						auto name = td->GetTypeName();
						tds.Add(name, td);
						writer << name;
					}
				}
			}

/***********************************************************************
LoadMetaonlyTypes
***********************************************************************/

			void LoadMetaonlyTypes(stream::IStream& inputStream)
			{
				ContextFreeReader reader(inputStream);
			}

#endif
		}
	}
}
