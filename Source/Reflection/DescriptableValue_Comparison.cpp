/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "DescriptableInterfaces.h"

namespace vl
{
	using namespace collections;

	namespace reflection
	{

/***********************************************************************
description::Value
***********************************************************************/

		namespace description
		{
			std::partial_ordering operator<=>(const Value& a, const Value& b)
			{
				auto avt = a.GetValueType();
				auto bvt = b.GetValueType();

				if (avt == Value::RawPtr || avt == Value::SharedPtr)
				{
					if (bvt == Value::RawPtr || bvt == Value::SharedPtr)
					{
						auto pa = a.GetRawPtr();
						auto pb = b.GetRawPtr();
						return pa <=> pb;
					}
				}

				if (avt != bvt)
				{
					return avt <=> bvt;
				}

				if (avt == Value::BoxedValue)
				{
					auto adt = a.GetTypeDescriptor();
					auto bdt = b.GetTypeDescriptor();
					if (adt == bdt)
					{
						auto pa = a.GetBoxedValue();
						auto pb = b.GetBoxedValue();
						switch (pa->ComparePrimitive(pb))
						{
						case IBoxedValue::Smaller: return std::partial_ordering::less;
						case IBoxedValue::Greater: return std::partial_ordering::greater;
						case IBoxedValue::Equal: return std::partial_ordering::equivalent;
						default: return std::partial_ordering::unordered;
						}
					}

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
					auto aSt = a.GetTypeDescriptor()->GetSerializableType();
					auto bSt = b.GetTypeDescriptor()->GetSerializableType();
					if (aSt)
					{
						if (bSt)
						{
							auto aSt = a.GetTypeDescriptor()->GetSerializableType();
							auto bSt = b.GetTypeDescriptor()->GetSerializableType();

							WString aText;
							WString bText;
							aSt->Serialize(a, aText);
							bSt->Serialize(b, bText);
							return aText <=> bText;
						}
						else
						{
							return std::strong_ordering::greater;
						}
					}
					else
					{
						if (bSt)
						{
							return std::strong_ordering::less;
						}
						else
						{
							if (a.GetTypeDescriptor() != b.GetTypeDescriptor())
							{
								auto aText = a.GetTypeDescriptor()->GetTypeName();
								auto bText = b.GetTypeDescriptor()->GetTypeName();
								return aText <=> bText;
							}

							switch (a.GetTypeDescriptor()->GetTypeDescriptorFlags())
							{
							case TypeDescriptorFlags::Struct:
								{
									auto td = a.GetTypeDescriptor();
									vint count = td->GetPropertyCount();
									for (vint i = 0; i < count; i++)
									{
										auto prop = td->GetProperty(i);
										auto ap = prop->GetValue(a);
										auto bp = prop->GetValue(b);
										auto r = ap <=> bp;
										if (r != 0) return r;
									}
								}
								return std::strong_ordering::equal;
							case TypeDescriptorFlags::FlagEnum:
							case TypeDescriptorFlags::NormalEnum:
								{
									auto ai = a.GetTypeDescriptor()->GetEnumType()->FromEnum(a);
									auto bi = a.GetTypeDescriptor()->GetEnumType()->FromEnum(b);
									return ai <=> bi;
								}
							default:
								return std::strong_ordering::equal;
							}
						}
					}
#else
					std::partial_ordering::unordered
#endif
				}

				return std::strong_ordering::equal;
			}
		}
	}
}
