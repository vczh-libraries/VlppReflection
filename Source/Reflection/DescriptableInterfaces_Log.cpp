/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "DescriptableInterfaces.h"
#include "./Reflection/Reflection.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			using namespace collections;

#ifndef VCZH_DEBUG_NO_REFLECTION

/***********************************************************************
LogTypeManager (attributes)
***********************************************************************/

			WString LogTypeManager_FormatAttribute(IAttributeInfo* info)
			{
				WString result = info->GetAttributeType()->GetTypeName() + L"(";
				for (vint i = 0; i < info->GetAttributeValueCount(); i++)
				{
					if (i > 0) result += L", ";
					auto value = info->GetAttributeValue(i);
					auto valueType = info->GetAttributeValueType(i);
					CHECK_ERROR(valueType != nullptr, L"vl::reflection::description::LogTypeManager_FormatAttribute(IAttributeInfo*)#Failed to resolve the reflected type of an attribute argument.");

					if (valueType->GetTypeName() == TypeInfo<ITypeDescriptor>::content.typeName)
					{
						if (value.GetValueType() == Value::Null)
						{
							result += valueType->GetTypeName() + L":null";
						}
						else
						{
							auto rawPtr = value.GetRawPtr();
							auto td = dynamic_cast<ITypeDescriptor*>(rawPtr);
							CHECK_ERROR(td != nullptr, L"vl::reflection::description::LogTypeManager_FormatAttribute(IAttributeInfo*)#ITypeDescriptor* attribute value must point to a valid ITypeDescriptor.");
							result += valueType->GetTypeName() + L":" + td->GetTypeName();
						}
					}
					else
					{
						auto serializableType = valueType->GetSerializableType();
						CHECK_ERROR(serializableType != nullptr, L"vl::reflection::description::LogTypeManager_FormatAttribute(IAttributeInfo*)#Attribute argument must use a serializable reflected type.");

						WString data;
						CHECK_ERROR(serializableType->Serialize(value, data), L"vl::reflection::description::LogTypeManager_FormatAttribute(IAttributeInfo*)#Failed to serialize an attribute argument.");
						result += valueType->GetTypeName() + L":" + data;
					}
				}
				result += L")";
				return result;
			}

			void LogTypeManager_PrintAttributes(stream::TextWriter& writer, IAttributeBag* bag, const WString& prefix)
			{
				for (vint i = 0; i < bag->GetAttributeCount(); i++)
				{
					writer.WriteLine(prefix + L"@" + LogTypeManager_FormatAttribute(bag->GetAttribute(i)));
				}
			}

			void LogTypeManager_PrintParameterAttributes(stream::TextWriter& writer, IMethodInfo* info)
			{
				for (vint i = 0; i < info->GetParameterCount(); i++)
				{
					auto parameter = info->GetParameter(i);
					for (vint j = 0; j < parameter->GetAttributeCount(); j++)
					{
						writer.WriteLine(L"    @(" + parameter->GetName() + L")" + LogTypeManager_FormatAttribute(parameter->GetAttribute(j)));
					}
				}
			}

/***********************************************************************
LogTypeManager (enum)
***********************************************************************/

			void LogTypeManager_Enum(stream::TextWriter& writer, ITypeDescriptor* type)
			{
				if (type->GetTypeDescriptorFlags() == TypeDescriptorFlags::FlagEnum)
				{
					writer.WriteLine(L"@Flags");
				}
				writer.WriteLine(L"enum " + type->GetTypeName() + L" {");

				auto enumType = type->GetEnumType();
				for (vint j = 0; j < enumType->GetItemCount(); j++)
				{
					writer.WriteLine(L"    " + enumType->GetItemName(j) + L" = " + u64tow(enumType->GetItemValue(j)) + L",");
				}

				writer.WriteLine(L"}");
			}

/***********************************************************************
LogTypeManager (struct)
***********************************************************************/

			void LogTypeManager_Property(stream::TextWriter& writer, IPropertyInfo* info)
			{
				LogTypeManager_PrintAttributes(writer, info, L"    ");
				if (auto cpp = info->GetCpp())
				{
					writer.WriteLine(L"    @ReferenceTemplate:" + cpp->GetReferenceTemplate());
				}
				writer.WriteString(L"    property " + info->GetName() + L" : " + info->GetReturn()->GetTypeFriendlyName());
				if (info->IsReadable() && info->IsWritable() && !info->GetGetter() && !info->GetSetter() && !info->GetValueChangedEvent())
				{
					writer.WriteLine(L";");
				}
				else
				{
					writer.WriteString(L" { ");
					if (info->IsReadable())
					{
						writer.WriteString(L"get");
						if (auto mi = info->GetGetter())
						{
							writer.WriteString(L":" + mi->GetName());
						}
						writer.WriteString(L" ");
					}
					if (info->IsWritable())
					{
						writer.WriteString(L"set");
						if (auto mi = info->GetSetter())
						{
							writer.WriteString(L":" + mi->GetName());
						}
						writer.WriteString(L" ");
					}
					if (auto ei = info->GetValueChangedEvent())
					{
						writer.WriteString(L"event:" + ei->GetName() + L" ");
					}
					writer.WriteLine(L"}");
				}
			}

			void LogTypeManager_Struct(stream::TextWriter& writer, ITypeDescriptor* type)
			{
				writer.WriteLine(L"struct " + type->GetTypeName() + L" {");
				for (vint j = 0; j<type->GetPropertyCount(); j++)
				{
					IPropertyInfo* info = type->GetProperty(j);
					LogTypeManager_Property(writer, info);
				}
				writer.WriteLine(L"}");
			}

/***********************************************************************
LogTypeManager (data)
***********************************************************************/

			void LogTypeManager_Data(stream::TextWriter& writer, ITypeDescriptor* type)
			{
				writer.WriteLine(L"primitive " + type->GetTypeName() + L";");
			}

/***********************************************************************
LogTypeManager (class)
***********************************************************************/

			void LogTypeManager_PrintEvents(stream::TextWriter& writer, ITypeDescriptor* type)
			{
				bool printed = false;
				for (vint j = 0; j < type->GetEventCount(); j++)
				{
					printed = true;
					IEventInfo* info = type->GetEvent(j);
					LogTypeManager_PrintAttributes(writer, info, L"    ");
					if (auto cpp = info->GetCpp())
					{
						writer.WriteLine(L"    @AttachTemplate:" + cpp->GetAttachTemplate());
						writer.WriteLine(L"    @DetachTemplate:" + cpp->GetDetachTemplate());
						writer.WriteLine(L"    @InvokeTemplate:" + cpp->GetInvokeTemplate());
					}

					writer.WriteString(L"    event " + info->GetName() + L" : " + info->GetHandlerType()->GetTypeFriendlyName());

					if (info->GetObservingPropertyCount() > 0)
					{
						writer.WriteLine(L" observing {");
						vint count = info->GetObservingPropertyCount();
						for (vint i = 0; i < count; i++)
						{
							writer.WriteLine(L"        " + info->GetObservingProperty(i)->GetName() + L",");
						}
						writer.WriteLine(L"    }");
					}
					else
					{
						writer.WriteLine(L";");
					}
				}
				if (printed)
				{
					writer.WriteLine(L"");
				}
			}

			void LogTypeManager_PrintProperties(stream::TextWriter& writer, ITypeDescriptor* type)
			{
				bool printed = false;
				for (vint j = 0; j < type->GetPropertyCount(); j++)
				{
					printed = true;
					IPropertyInfo* info = type->GetProperty(j);
					LogTypeManager_Property(writer, info);
				}
				if (printed)
				{
					writer.WriteLine(L"");
				}
			}

			void LogTypeManager_Method(stream::TextWriter& writer, IMethodInfo* info, const wchar_t* title)
			{
				LogTypeManager_PrintAttributes(writer, info, L"    ");
				if (auto cpp = info->GetCpp())
				{
					writer.WriteLine(L"    @InvokeTemplate:" + cpp->GetInvokeTemplate());
					writer.WriteLine(L"    @ClosureTemplate:" + cpp->GetClosureTemplate());
				}
				LogTypeManager_PrintParameterAttributes(writer, info);

				writer.WriteString(L"    ");
				writer.WriteString(title);
				writer.WriteString(L" " + info->GetName() + L"(");
				for (vint l = 0; l < info->GetParameterCount(); l++)
				{
					if (l > 0) writer.WriteString(L", ");
					IParameterInfo* parameter = info->GetParameter(l);
					writer.WriteString(parameter->GetName() + L" : " + parameter->GetType()->GetTypeFriendlyName());
				}
				writer.WriteLine(L") : " + info->GetReturn()->GetTypeFriendlyName() + L";");
			}

			void LogTypeManager_PrintMethods(stream::TextWriter& writer, ITypeDescriptor* type)
			{
				bool printed = false;
				for (vint j = 0; j < type->GetMethodGroupCount(); j++)
				{
					IMethodGroupInfo* group = type->GetMethodGroup(j);
					for (vint k = 0; k < group->GetMethodCount(); k++)
					{
						printed = true;
						IMethodInfo* info = group->GetMethod(k);
						if (info->IsStatic())
						{
							LogTypeManager_Method(writer, info, L"static function");
						}
						else
						{
							LogTypeManager_Method(writer, info, L"function");
						}
					}
				}
				if (printed)
				{
					writer.WriteLine(L"");
				}
			}

			void LogTypeManager_PrintConstructors(stream::TextWriter& writer, ITypeDescriptor* type)
			{
				if (IMethodGroupInfo* group = type->GetConstructorGroup())
				{
					for (vint k = 0; k < group->GetMethodCount(); k++)
					{
						IMethodInfo* info = group->GetMethod(k);
						LogTypeManager_Method(writer, info, L"constructor");
					}
				}
			}

			void LogTypeManager_Class(stream::TextWriter& writer, ITypeDescriptor* type)
			{
				bool acceptProxy = false;
				bool isInterface = (type->GetTypeDescriptorFlags() & TypeDescriptorFlags::InterfaceType) != TypeDescriptorFlags::Undefined;
				writer.WriteString((isInterface ? L"interface " : L"class ") + type->GetTypeName());
				for (vint j = 0; j<type->GetBaseTypeDescriptorCount(); j++)
				{
					writer.WriteString(j == 0 ? L" : " : L", ");
					writer.WriteString(type->GetBaseTypeDescriptor(j)->GetTypeName());
				}
				writer.WriteLine(L" {");

				LogTypeManager_PrintEvents(writer, type);
				LogTypeManager_PrintProperties(writer, type);
				LogTypeManager_PrintMethods(writer, type);
				LogTypeManager_PrintConstructors(writer, type);

				writer.WriteLine(L"}");
			}

/***********************************************************************
LogTypeManager
***********************************************************************/

			bool IsInterfaceType(ITypeDescriptor* typeDescriptor, bool& acceptProxy)
			{
				bool containsConstructor = false;
				if (IMethodGroupInfo* group = typeDescriptor->GetConstructorGroup())
				{
					containsConstructor = group->GetMethodCount() > 0;
					if (group->GetMethodCount() == 1)
					{
						if (IMethodInfo* info = group->GetMethod(0))
						{
							if (info->GetParameterCount() == 1 && info->GetParameter(0)->GetType()->GetTypeDescriptor()->GetTypeName() == TypeInfo<IValueInterfaceProxy>::content.typeName)
							{
								acceptProxy = true;
								return true;
							}
						}
					}
				}

				if (!containsConstructor)
				{
					if (typeDescriptor->GetTypeName() == TypeInfo<IDescriptable>::content.typeName)
					{
						return true;
					}
					else
					{
						for (vint i = 0; i < typeDescriptor->GetBaseTypeDescriptorCount(); i++)
						{
							bool _acceptProxy = false;
							if (!IsInterfaceType(typeDescriptor->GetBaseTypeDescriptor(i), _acceptProxy))
							{
								return false;
							}
						}
						const wchar_t* name = typeDescriptor->GetTypeName().Buffer();
						while (const wchar_t* next = ::wcschr(name, L':'))
						{
							name = next + 1;
						}
						return name[0] == L'I' && (L'A' <= name[1] && name[1] <= L'Z');
					}
				}
				return false;
			}

			void LogTypeManager(stream::TextWriter& writer)
			{
				for (vint i = 0; i < GetGlobalTypeManager()->GetTypeDescriptorCount(); i++)
				{
					ITypeDescriptor* type = GetGlobalTypeManager()->GetTypeDescriptor(i);
					if (auto cpp = type->GetCpp())
					{
						writer.WriteLine(L"@FullName:" + cpp->GetFullName());
					}
					if (type->GetValueType())
					{
						writer.WriteLine(L"@ValueType");
					}
					if (type->GetSerializableType())
					{
						writer.WriteLine(L"@Serializable");
					}
					LogTypeManager_PrintAttributes(writer, type, L"");

					switch (type->GetTypeDescriptorFlags())
					{
					case TypeDescriptorFlags::Object:
					case TypeDescriptorFlags::IDescriptable:
					case TypeDescriptorFlags::Class:
					case TypeDescriptorFlags::Interface:
						LogTypeManager_Class(writer, type);
						break;
					case TypeDescriptorFlags::FlagEnum:
					case TypeDescriptorFlags::NormalEnum:
						LogTypeManager_Enum(writer, type);
						break;
					case TypeDescriptorFlags::Primitive:
						LogTypeManager_Data(writer, type);
						break;
					case TypeDescriptorFlags::Struct:
						LogTypeManager_Struct(writer, type);
						break;
					default:;
					}
					writer.WriteLine(L"");
				}
			}
#endif
		}
	}
}
