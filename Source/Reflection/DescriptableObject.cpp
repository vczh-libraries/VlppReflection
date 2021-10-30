/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "DescriptableObject.h"

namespace vl
{
	namespace reflection
	{
/***********************************************************************
DescriptableObject
***********************************************************************/

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

		bool DescriptableObject::IsAggregated()
		{
			return aggregationInfo != nullptr;
		}

		vint DescriptableObject::GetAggregationSize()
		{
			CHECK_ERROR(IsAggregated(), L"vl::reflection::DescriptableObject::GetAggregationSize()#This function should not be called on non-aggregated objects.");
			return aggregationSize;
		}

		DescriptableObject* DescriptableObject::GetAggregationRoot()
		{
			CHECK_ERROR(IsAggregated(), L"vl::reflection::DescriptableObject::GetAggregationRoot()#This function should not be called on non-aggregated objects.");
			return aggregationInfo[aggregationSize];
		}

		void DescriptableObject::SetAggregationRoot(DescriptableObject* value)
		{
			CHECK_ERROR(value != nullptr, L"vl::reflection::DescriptableObject::SetAggregationRoot(Descriptable*)#The root object should not null.");
			CHECK_ERROR(value->IsAggregated() && value->GetAggregationRoot() == nullptr, L"vl::reflection::DescriptableObject::SetAggregationRoot(Descriptable*)#The root object should not have an aggregation root.");
			if (!IsAggregated())
			{
				InitializeAggregation(0);
			}
			aggregationInfo[aggregationSize] = value;
			aggregationInfo[aggregationSize + 1] = value;
			for (vint i = 0; i < aggregationSize; i++)
			{
				if (aggregationInfo[i])
				{
					aggregationInfo[i]->SetAggregationRoot(value);
				}
			}
		}

		DescriptableObject* DescriptableObject::GetAggregationParent(vint index)
		{
			CHECK_ERROR(IsAggregated(), L"vl::reflection::DescriptableObject::GetAggregationParent(vint)#This function should not be called on non-aggregated objects.");
			CHECK_ERROR(0 <= index && index < aggregationSize, L"vl::reflection::DescriptableObject::GetAggregationParent(vint)#Index out of range.");
			return aggregationInfo[index];
		}

		void DescriptableObject::SetAggregationParent(vint index, DescriptableObject* value)
		{
			CHECK_ERROR(IsAggregated(), L"vl::reflection::DescriptableObject::SetAggregationParent(vint, DescriptableObject*)#This function should not be called on non-aggregated objects.");
			CHECK_ERROR(0 <= index && index < aggregationSize, L"vl::reflection::DescriptableObject::SetAggregationParent(vint, DescriptableObject*)#Index out of range.");
			CHECK_ERROR(aggregationInfo[index] == nullptr, L"vl::reflection::DescriptableObject::SetAggregationParent(vint, DescriptableObject*)#This index has been used.");
			CHECK_ERROR(value != nullptr, L"vl::reflection::DescriptableObject::SetAggregationParent(vint, DescriptableObject*)#Parent should not be null.");
			CHECK_ERROR(!value->IsAggregated() || value->GetAggregationRoot() == nullptr, L"vl::reflection::DescriptableObject::SetAggregationParent(vint, DescriptableObject*)#Parent already has a aggregation root.");
			CHECK_ERROR(value->referenceCounter == 0, L"vl::reflection::DescriptableObject::SetAggregationParent(vint, DescriptableObject*)#Parent should not be contained in any smart pointer.");
			value->SetAggregationRoot(this);
			aggregationInfo[index] = value;
		}

		void DescriptableObject::SetAggregationParent(vint index, Ptr<DescriptableObject>& value)
		{
			CHECK_ERROR(IsAggregated(), L"vl::reflection::DescriptableObject::SetAggregationParent(vint, Ptr<DescriptableObject>&)#This function should not be called on non-aggregated objects.");
			CHECK_ERROR(0 <= index && index < aggregationSize, L"vl::reflection::DescriptableObject::SetAggregationParent(vint, Ptr<DescriptableObject>&)#Index out of range.");
			CHECK_ERROR(aggregationInfo[index] == nullptr, L"vl::reflection::DescriptableObject::SetAggregationParent(vint, Ptr<DescriptableObject>&)#This index has been used.");
			CHECK_ERROR(value, L"vl::reflection::DescriptableObject::SetAggregationParent(vint, Ptr<DescriptableObject>&)#Parent should not be null");
			CHECK_ERROR(!value->IsAggregated() || value->GetAggregationRoot() == nullptr, L"vl::reflection::DescriptableObject::SetAggregationParent(vint, Ptr<DescriptableObject>&)#Parent already has a aggregation root.");
			CHECK_ERROR(value->referenceCounter == 1, L"vl::reflection::DescriptableObject::SetAggregationParent(vint, Ptr<DescriptableObject>&)#Parent should not be contained in any other smart pointer.");
			value->SetAggregationRoot(this);

			auto parent = value.Detach();
			aggregationInfo[index] = parent;
		}

		void DescriptableObject::InitializeAggregation(vint size)
		{
			CHECK_ERROR(!IsAggregated(), L"vl::reflection::DescriptableObject::InitializeAggregation(vint)#This function should not be called on aggregated objects.");
			CHECK_ERROR(size >= 0, L"vl::reflection::DescriptableObject::InitializeAggregation(vint)#Size shout not be negative.");
			aggregationSize = size;
			aggregationInfo = new DescriptableObject * [size + 2];
			memset(aggregationInfo, 0, sizeof(*aggregationInfo) * (size + 2));
		}
#endif

		void DescriptableObject::FinalizeAggregation()
		{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			if (IsAggregated())
			{
				if (auto root = GetAggregationRoot())
				{
					if (aggregationInfo[aggregationSize + 1] == nullptr)
					{
						return;
					}
					else
					{
						aggregationInfo[aggregationSize + 1] = nullptr;
					}

					if (!root->destructing)
					{
						destructing = true;
						delete root;
					}
				}
			}
#endif
		}

		DescriptableObject::DescriptableObject()
			:referenceCounter(0)
			, sharedPtrDestructorProc(0)
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			, objectSize(0)
			, typeDescriptor(0)
			, destructing(false)
			, aggregationInfo(nullptr)
			, aggregationSize(-1)
#endif
		{
		}

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexceptions"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wterminate"
#endif
		DescriptableObject::~DescriptableObject()
		{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			destructing = true;
			if (IsAggregated())
			{
				if (auto root = GetAggregationRoot())
				{
					if (aggregationInfo[aggregationSize + 1] != nullptr)
					{
#pragma warning (push)
#pragma warning (disable: 4297)
						// Your class should call FinalizeAggregation in the destructor if it inherits from AggregatableDescription<T>.
						CHECK_ERROR(!IsAggregated(), L"vl::reflection::DescriptableObject::~DescriptableObject0()#FinalizeAggregation function should be called.");
#pragma warning (pop)
					}
				}
				for (vint i = 0; i < aggregationSize; i++)
				{
					if (auto parent = GetAggregationParent(i))
					{
						if (!parent->destructing)
						{
							delete parent;
						}
					}
				}
				delete[] aggregationInfo;
			}
#endif
		}
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

		description::ITypeDescriptor* DescriptableObject::GetTypeDescriptor()
		{
			return typeDescriptor ? *typeDescriptor : 0;
		}

#endif

		Ptr<Object> DescriptableObject::GetInternalProperty(const WString& name)
		{
			if (!internalProperties) return 0;
			vint index = internalProperties->Keys().IndexOf(name);
			if (index == -1) return 0;
			return internalProperties->Values().Get(index);
		}

		void DescriptableObject::SetInternalProperty(const WString& name, Ptr<Object> value)
		{
			if (internalProperties)
			{
				vint index = internalProperties->Keys().IndexOf(name);
				if (index == -1)
				{
					if (value)
					{
						internalProperties->Add(name, value);
					}
				}
				else
				{
					if (value)
					{
						internalProperties->Set(name, value);
					}
					else
					{
						internalProperties->Remove(name);
						if (internalProperties->Count() == 0)
						{
							internalProperties = 0;
						}
					}
				}
			}
			else
			{
				if (value)
				{
					internalProperties = new InternalPropertyMap;
					internalProperties->Add(name, value);
				}
			}
		}

		bool DescriptableObject::Dispose(bool forceDisposing)
		{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			if (IsAggregated())
			{
				if (auto root = GetAggregationRoot())
				{
					return root->Dispose(forceDisposing);
				}
			}
#endif

			if (referenceCounter > 0 && forceDisposing)
			{
				throw description::ValueNotDisposableException();
			}

			if (sharedPtrDestructorProc)
			{
				return sharedPtrDestructorProc(this, forceDisposing);
			}
			else
			{
				delete this;
				return true;
			}
		}

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

		DescriptableObject* DescriptableObject::SafeGetAggregationRoot()
		{
			if (IsAggregated())
			{
				if (auto root = GetAggregationRoot())
				{
					return root;
				}
			}
			return this;
		}

#endif
	}
}
