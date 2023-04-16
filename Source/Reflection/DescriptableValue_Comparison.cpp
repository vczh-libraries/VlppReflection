/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "DescriptableInterfaces.h"
#include "Boxing/BoxingValue.h"

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
			namespace pbt_selector
			{
				template<typename T>
				struct UnboxTypeBase { using Type = T; };

				template<PredefinedBoxableType> struct RealUnboxType;
				template<PredefinedBoxableType> struct ExpectedUnboxType;

				template<> struct RealUnboxType<PredefinedBoxableType::PBT_S8>	: UnboxTypeBase<vint64_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_S16>	: UnboxTypeBase<vint64_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_S32>	: UnboxTypeBase<vint64_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_S64>	: UnboxTypeBase<vint64_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_U8>	: UnboxTypeBase<vuint64_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_U16>	: UnboxTypeBase<vuint64_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_U32>	: UnboxTypeBase<vuint64_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_U64>	: UnboxTypeBase<vuint64_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_F32>	: UnboxTypeBase<double> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_F64>	: UnboxTypeBase<double> {};

				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_S8>	: UnboxTypeBase<vint8_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_S16>	: UnboxTypeBase<vint16_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_S32>	: UnboxTypeBase<vint32_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_S64>	: UnboxTypeBase<vint64_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_U8>	: UnboxTypeBase<vuint8_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_U16>	: UnboxTypeBase<vuint16_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_U32>	: UnboxTypeBase<vuint32_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_U64>	: UnboxTypeBase<vuint64_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_F32>	: UnboxTypeBase<float> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_F64>	: UnboxTypeBase<double> {};

				template<PredefinedBoxableType PBT>
				typename ExpectedUnboxType<PBT>::Type UnboxForComparison(const Value& v)
				{
					return (typename ExpectedUnboxType<PBT>::Type)UnboxValue<typename RealUnboxType<PBT>::Type>(v);
				}

#define DEFINE_PBT_COMPARE(TYPE1, TYPE2)\
				inline std::partial_ordering Compare(TYPE1& v1, TYPE2& v2)\
				{\
					return v1 <=> v2;\
				}\

				DEFINE_PBT_COMPARE(vint64_t, vint64_t)
				DEFINE_PBT_COMPARE(vuint64_t, vuint64_t)
				DEFINE_PBT_COMPARE(double, double)
				DEFINE_PBT_COMPARE(vint64_t, double)
				DEFINE_PBT_COMPARE(vuint64_t, double)
				DEFINE_PBT_COMPARE(double, vint64_t)
				DEFINE_PBT_COMPARE(double, vuint64_t)
					
				inline std::partial_ordering Compare(vint64_t& v1, vuint64_t& v2)
				{
					if (v2 > _I64_MAX) return std::partial_ordering::less;
					return v1 <=> (vint64_t)v2;
				}

				inline std::partial_ordering Compare(vuint64_t& v1, vint64_t& v2)
				{
					if (v1 > _I64_MAX) return std::partial_ordering::greater;
					return (vint64_t)v1 <=> v2;
				}

				template<PredefinedBoxableType PBT1, PredefinedBoxableType PBT2>
				std::partial_ordering PBT_Compare(const Value& v1, const Value& v2)
				{
					using E1 = typename ExpectedUnboxType<PBT1>::Type;
					using E2 = typename ExpectedUnboxType<PBT2>::Type;
					using R1 = typename RealUnboxType<PBT1>::Type;
					using R2 = typename RealUnboxType<PBT2>::Type;

					E1 e1 = (E1)UnboxValue<R1>(v1);
					E2 e2 = (E2)UnboxValue<R2>(v2);
					return Compare(e1, e2);
				}

#undef DEFINE_PBT_COMPARE
			}

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
