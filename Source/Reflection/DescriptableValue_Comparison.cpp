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
			namespace pbt_selector
			{
				template<typename T>
				struct UnboxTypeBase { using Type = T; };

				template<PredefinedBoxableType> struct ExpectedUnboxType;
				template<PredefinedBoxableType> struct RealUnboxType;

				constexpr vint PBT_MIN = (vint)PredefinedBoxableType::PBT_S8;
				constexpr vint PBT_MAX = (vint)PredefinedBoxableType::PBT_F64;
				constexpr vint PBT_COUNT = PBT_MAX - PBT_MIN + 1;

				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_S8>	: UnboxTypeBase<vint64_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_S16>	: UnboxTypeBase<vint64_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_S32>	: UnboxTypeBase<vint64_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_S64>	: UnboxTypeBase<vint64_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_U8>	: UnboxTypeBase<vuint64_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_U16>	: UnboxTypeBase<vuint64_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_U32>	: UnboxTypeBase<vuint64_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_U64>	: UnboxTypeBase<vuint64_t> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_F32>	: UnboxTypeBase<double> {};
				template<> struct ExpectedUnboxType<PredefinedBoxableType::PBT_F64>	: UnboxTypeBase<double> {};

				template<> struct RealUnboxType<PredefinedBoxableType::PBT_S8>	: UnboxTypeBase<vint8_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_S16>	: UnboxTypeBase<vint16_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_S32>	: UnboxTypeBase<vint32_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_S64>	: UnboxTypeBase<vint64_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_U8>	: UnboxTypeBase<vuint8_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_U16>	: UnboxTypeBase<vuint16_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_U32>	: UnboxTypeBase<vuint32_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_U64>	: UnboxTypeBase<vuint64_t> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_F32>	: UnboxTypeBase<float> {};
				template<> struct RealUnboxType<PredefinedBoxableType::PBT_F64>	: UnboxTypeBase<double> {};

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

					E1 e1 = (E1)v1.GetBoxedValue().Cast<IValueType::TypedBox<R1>>()->value;
					E2 e2 = (E2)v2.GetBoxedValue().Cast<IValueType::TypedBox<R2>>()->value;
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


#define DEFINE_PBT_MATRIX2(PBT1, PBT2) &pbt_selector::PBT_Compare<(PredefinedBoxableType)PBT1, (PredefinedBoxableType)PBT2>

#define DEFINE_PBT_MATRIX1(PBT1)\
			DEFINE_PBT_MATRIX2(PBT1, 0),\
			DEFINE_PBT_MATRIX2(PBT1, 1),\
			DEFINE_PBT_MATRIX2(PBT1, 2),\
			DEFINE_PBT_MATRIX2(PBT1, 3),\
			DEFINE_PBT_MATRIX2(PBT1, 4),\
			DEFINE_PBT_MATRIX2(PBT1, 5),\
			DEFINE_PBT_MATRIX2(PBT1, 6),\
			DEFINE_PBT_MATRIX2(PBT1, 7),\
			DEFINE_PBT_MATRIX2(PBT1, 8),\
			DEFINE_PBT_MATRIX2(PBT1, 9)

#define DEFINE_PBT_MATRIX\
			DEFINE_PBT_MATRIX1(0),\
			DEFINE_PBT_MATRIX1(1),\
			DEFINE_PBT_MATRIX1(2),\
			DEFINE_PBT_MATRIX1(3),\
			DEFINE_PBT_MATRIX1(4),\
			DEFINE_PBT_MATRIX1(5),\
			DEFINE_PBT_MATRIX1(6),\
			DEFINE_PBT_MATRIX1(7),\
			DEFINE_PBT_MATRIX1(8),\
			DEFINE_PBT_MATRIX1(9)

					{
						static std::partial_ordering(*PBT_CompareMatrix[pbt_selector::PBT_COUNT][pbt_selector::PBT_COUNT])(const Value & v1, const Value & v2) = { DEFINE_PBT_MATRIX };
						auto apbt = (vint)a.GetBoxedValue()->GetBoxableType();
						auto bpbt = (vint)b.GetBoxedValue()->GetBoxableType();
						if (pbt_selector::PBT_MIN <= apbt && apbt <= pbt_selector::PBT_MAX)
						{
							if (pbt_selector::PBT_MIN <= bpbt && bpbt <= pbt_selector::PBT_MAX)
							{
								return PBT_CompareMatrix[apbt][bpbt](a, b);
							}
						}
					}

#undef DEFINE_PBT_MATRIX
#undef DEFINE_PBT_MATRIX1
#undef DEFINE_PBT_MATRI2

					std::partial_ordering::unordered;
				}

				return std::partial_ordering::equivalent;
			}
		}
	}
}
