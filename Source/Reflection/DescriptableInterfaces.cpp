/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "DescriptableInterfaces.h"
#include "./Reflection/Reflection.h"

namespace vl
{
	using namespace collections;

	namespace reflection
	{
		namespace description
		{
/***********************************************************************
description::TypeManager
***********************************************************************/

#ifndef VCZH_DEBUG_NO_REFLECTION

			class TypeManager : public Object, public ITypeManager
			{
			public:
				static vint										typeVersion;

			protected:
				Dictionary<WString, Ptr<ITypeDescriptor>>		typeDescriptors;
				List<Ptr<ITypeLoader>>							typeLoaders;
				ITypeDescriptor*								rootType = nullptr;
				bool											loaded = false;

			public:
				TypeManager()
				{
				}

				~TypeManager()
				{
					Unload();
				}

				vint GetTypeDescriptorCount()override
				{
					return typeDescriptors.Values().Count();
				}

				ITypeDescriptor* GetTypeDescriptor(vint index)override
				{
					return typeDescriptors.Values().Get(index).Obj();
				}

				ITypeDescriptor* GetTypeDescriptor(const WString& name)override
				{
					vint index = typeDescriptors.Keys().IndexOf(name);
					return index == -1 ? 0 : typeDescriptors.Values().Get(index).Obj();
				}

				bool SetTypeDescriptor(const WString& name, Ptr<ITypeDescriptor> typeDescriptor)override
				{
					if (typeDescriptor && name != typeDescriptor->GetTypeName())
					{
						return false;
					}
					if (!typeDescriptors.Keys().Contains(name))
					{
						if (typeDescriptor)
						{
							typeDescriptors.Add(name, typeDescriptor);
							typeVersion++;
							return true;
						}
					}
					else
					{
						if (!typeDescriptor)
						{
							typeDescriptors.Remove(name);
							typeVersion++;
							return true;
						}
					}
					return false;
				}

				bool AddTypeLoader(Ptr<ITypeLoader> typeLoader)override
				{
					vint index = typeLoaders.IndexOf(typeLoader.Obj());
					if (index == -1)
					{
						typeLoaders.Add(typeLoader);
						if (loaded)
						{
							auto oldTypeVersion = typeVersion;
							typeLoader->Load(this);
							typeVersion = oldTypeVersion + 1;
						}
						return true;
					}
					else
					{
						return false;
					}
				}

				bool RemoveTypeLoader(Ptr<ITypeLoader> typeLoader)override
				{
					vint index = typeLoaders.IndexOf(typeLoader.Obj());
					if (index != -1)
					{
						if (loaded)
						{
							auto oldTypeVersion = typeVersion;
							typeLoader->Unload(this);
							typeVersion = oldTypeVersion + 1;
						}
						typeLoaders.RemoveAt(index);
						return true;
					}
					else
					{
						return false;
					}
				}

				bool Load()override
				{
					if (!loaded)
					{
						loaded = true;
						auto oldTypeVersion = typeVersion;
						// TODO: (enumerable) foreach
						for (vint i = 0; i < typeLoaders.Count(); i++)
						{
							typeLoaders[i]->Load(this);
						}
						typeVersion = oldTypeVersion + 1;
						return true;
					}
					else
					{
						return false;
					}
				}

				bool Unload()override
				{
					if (loaded)
					{
						loaded = false;
						rootType = 0;
						auto oldTypeVersion = typeVersion;
						// TODO: (enumerable) foreach
						for (vint i = 0; i < typeLoaders.Count(); i++)
						{
							typeLoaders[i]->Unload(this);
						}
						typeVersion = oldTypeVersion + 1;
						typeDescriptors.Clear();
						return true;
					}
					else
					{
						return false;
					}
				}

				bool Reload()override
				{
					Unload();
					Load();
					return true;
				}

				bool IsLoaded()override
				{
					return loaded;
				}

				ITypeDescriptor* GetRootType()override
				{
					if (!rootType)
					{
						rootType = description::GetTypeDescriptor<Value>();
					}
					return rootType;
				}

				vint GetTypeVersion() override
				{
					return typeVersion;
				}
			};
			vint TypeManager::typeVersion = -1;

/***********************************************************************
description::TypeManager helper functions
***********************************************************************/

			ITypeManager* globalTypeManager=0;
			bool initializedGlobalTypeManager=false;

			ITypeManager* GetGlobalTypeManager()
			{
				if (!initializedGlobalTypeManager)
				{
					initializedGlobalTypeManager = true;
					globalTypeManager = new TypeManager;
				}
				return globalTypeManager;
			}

			bool DestroyGlobalTypeManager()
			{
				if (initializedGlobalTypeManager && globalTypeManager)
				{
					delete globalTypeManager;
					globalTypeManager = nullptr;
					TypeManager::typeVersion++;
					return true;
				}
				else
				{
					return false;
				}
			}

			bool ResetGlobalTypeManager()
			{
				if (!DestroyGlobalTypeManager()) return false;
				initializedGlobalTypeManager = false;
				return true;
			}

			ITypeDescriptor* GetTypeDescriptor(const WString& name)
			{
				if (globalTypeManager)
				{
					if (!globalTypeManager->IsLoaded())
					{
						globalTypeManager->Load();
					}
					return globalTypeManager->GetTypeDescriptor(name);
				}
				return nullptr;
			}

/***********************************************************************
Cpp Helper Functions
***********************************************************************/

			WString CppGetFullName(ITypeDescriptor* type)
			{
				if (auto cpp = type->GetCpp())
				{
					if (cpp->GetFullName() == L"void" || cpp->GetFullName() == L"vl::reflection::description::VoidValue")
					{
						return L"void";
					}
					else if (cpp->GetFullName() == L"float")
					{
						return L"float";
					}
					else if (cpp->GetFullName() == L"double")
					{
						return L"double";
					}
					else if (cpp->GetFullName() == L"bool")
					{
						return L"bool";
					}
					else if (cpp->GetFullName() == L"wchar_t")
					{
						return L"wchar_t";
					}
					else
					{
						return L"::" + cpp->GetFullName();
					}
				}
				else
				{
					return L"::vl::" + type->GetTypeName();
				}
			}

			WString CppGetReferenceTemplate(IPropertyInfo* prop)
			{
				if (auto cpp = prop->GetCpp())
				{
					return cpp->GetReferenceTemplate();
				}
				else if ((prop->GetOwnerTypeDescriptor()->GetTypeDescriptorFlags() & TypeDescriptorFlags::ReferenceType) != TypeDescriptorFlags::Undefined)
				{
					return WString::Unmanaged(L"$This->$Name");
				}
				else
				{
					return WString::Unmanaged(L"$This.$Name");
				}
			}

			WString CppGetClosureTemplate(IMethodInfo* method)
			{
				if (auto cpp = method->GetCpp())
				{
					return cpp->GetClosureTemplate();
				}

				if (method->IsStatic())
				{
					return WString::Unmanaged(L"::vl::Func<$Func>(&$Type::$Name)");
				}
				else
				{
					return WString::Unmanaged(L"::vl::Func<$Func>($This, &$Type::$Name)");
				}
			}

			WString CppGetInvokeTemplate(IMethodInfo* method)
			{
				if (auto cpp = method->GetCpp())
				{
					return cpp->GetInvokeTemplate();
				}

				if (method->GetOwnerMethodGroup() == method->GetOwnerTypeDescriptor()->GetConstructorGroup())
				{
					return WString::Unmanaged(L"new $Type($Arguments)");
				}
				else if (method->IsStatic())
				{
					return WString::Unmanaged(L"$Type::$Name($Arguments)");
				}
				else
				{
					return WString::Unmanaged(L"$This->$Name($Arguments)");
				}
			}

			WString CppGetAttachTemplate(IEventInfo* ev)
			{
				auto cpp = ev->GetCpp();
				return cpp == nullptr ? WString::Unmanaged(L"::vl::__vwsn::EventAttach($This->$Name, $Handler)") : cpp->GetAttachTemplate();
			}

			WString CppGetDetachTemplate(IEventInfo* ev)
			{
				auto cpp = ev->GetCpp();
				return cpp == nullptr ? WString::Unmanaged(L"::vl::__vwsn::EventDetach($This->$Name, $Handler)") : cpp->GetDetachTemplate();
			}

			WString CppGetInvokeTemplate(IEventInfo* ev)
			{
				auto cpp = ev->GetCpp();
				return cpp == nullptr ? WString::Unmanaged(L"::vl::__vwsn::EventInvoke($This->$Name)($Arguments)") : cpp->GetInvokeTemplate();
			}

			bool CppExists(ITypeDescriptor* type)
			{
				auto cpp = type->GetCpp();
				return cpp == nullptr || cpp->GetFullName() != L"*";
			}

			bool CppExists(IPropertyInfo* prop)
			{
				if (auto cpp = prop->GetCpp())
				{
					return cpp->GetReferenceTemplate() != L"*";
				}
				else if (auto method = prop->GetGetter())
				{
					return CppExists(method);
				}
				else
				{
					return true;
				}
			}

			bool CppExists(IMethodInfo* method)
			{
				auto cpp = method->GetCpp();
				return cpp == nullptr || cpp->GetInvokeTemplate() != L"*";
			}

			bool CppExists(IEventInfo* ev)
			{
				auto cpp = ev->GetCpp();
				return cpp == nullptr || cpp->GetInvokeTemplate() != L"*";
			}

#endif
		}
	}
}
