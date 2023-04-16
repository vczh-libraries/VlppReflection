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
			std::strong_ordering operator<=>(const Value& a, const Value& b)
			{
				switch (a.GetValueType())
				{
				case Value::RawPtr:
				case Value::SharedPtr:
					switch (b.GetValueType())
					{
					case Value::RawPtr:
					case Value::SharedPtr:
						{
							auto pa = a.GetRawPtr();
							auto pb = b.GetRawPtr();
							return pa <=> pb;
						}
					case Value::BoxedValue:
						return std::strong_ordering::less;
					default:
						return std::strong_ordering::greater;
					}
				case Value::BoxedValue:
					switch (b.GetValueType())
					{
					case Value::RawPtr:
					case Value::SharedPtr:
						return std::strong_ordering::greater;
					case Value::BoxedValue:
						{
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
								auto pa = a.GetBoxedValue();
								auto pb = b.GetBoxedValue();
								switch (pa->ComparePrimitive(pb))
								{
								case IBoxedValue::Smaller: return std::strong_ordering::less;
								case IBoxedValue::Greater: return std::strong_ordering::greater;
								case IBoxedValue::Equal: return std::strong_ordering::equal;
								default:;
								}
								return pa.Obj() <=> pb.Obj();
#endif
						}
					default:
						return std::strong_ordering::greater;
					}
				default:
					switch (b.GetValueType())
					{
					case Value::RawPtr:
					case Value::SharedPtr:
					case Value::BoxedValue:
						return std::strong_ordering::less;
					default:
						return std::strong_ordering::equal;
					}
				}
			}
		}
	}
}
