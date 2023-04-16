/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "TypedValueSerializerProvider.h"
#include <VlppRegex.h>

namespace vl
{
	using namespace regex;

	namespace reflection
	{
		namespace description
		{
/***********************************************************************
wchar_t
***********************************************************************/

			wchar_t TypedValueSerializerProvider<wchar_t>::GetDefaultValue()
			{
				return 0;
			}

			bool TypedValueSerializerProvider<wchar_t>::Serialize(const wchar_t& input, WString& output)
			{
				output = input ? WString::FromChar(input) : L"";
				return true;
			}

			bool TypedValueSerializerProvider<wchar_t>::Deserialize(const WString& input, wchar_t& output)
			{
				if (input.Length() > 1) return false;
				output = input.Length() == 0 ? 0 : input[0];
				return true;
			}

/***********************************************************************
bool
***********************************************************************/

			WString TypedValueSerializerProvider<WString>::GetDefaultValue()
			{
				return L"";
			}

			bool TypedValueSerializerProvider<WString>::Serialize(const WString& input, WString& output)
			{
				output = input;
				return true;
			}

			bool TypedValueSerializerProvider<WString>::Deserialize(const WString& input, WString& output)
			{
				output = input;
				return true;
			}

/***********************************************************************
WString
***********************************************************************/

			bool TypedValueSerializerProvider<bool>::GetDefaultValue()
			{
				return false;
			}

			bool TypedValueSerializerProvider<bool>::Serialize(const bool& input, WString& output)
			{
				output = input ? L"true" : L"false";
				return true;
			}

			bool TypedValueSerializerProvider<bool>::Deserialize(const WString& input, bool& output)
			{
				output = input == L"true";
				return input == L"true" || input == L"false";
			}

/***********************************************************************
Locale
***********************************************************************/

			Locale TypedValueSerializerProvider<Locale>::GetDefaultValue()
			{
				return Locale();
			}

			bool TypedValueSerializerProvider<Locale>::Serialize(const Locale& input, WString& output)
			{
				output = input.GetName();
				return true;
			}

			bool TypedValueSerializerProvider<Locale>::Deserialize(const WString& input, Locale& output)
			{
				output = Locale(input);
				return true;
			}

/***********************************************************************
DateTime
***********************************************************************/

			BEGIN_GLOBAL_STORAGE_CLASS(DateTimeSerializerStorage)
				Regex* regexDateTime = nullptr;
				vint _Y, _M, _D, _h, _m, _s, _ms;

				INITIALIZE_GLOBAL_STORAGE_CLASS
					regexDateTime = new Regex(L"(<Y>/d/d/d/d)-(<M>/d/d)-(<D>/d/d) (<h>/d/d):(<m>/d/d):(<s>/d/d).(<ms>/d/d/d)");
					_Y = regexDateTime->CaptureNames().IndexOf(L"Y");
					_M = regexDateTime->CaptureNames().IndexOf(L"M");
					_D = regexDateTime->CaptureNames().IndexOf(L"D");
					_h = regexDateTime->CaptureNames().IndexOf(L"h");
					_m = regexDateTime->CaptureNames().IndexOf(L"m");
					_s = regexDateTime->CaptureNames().IndexOf(L"s");
					_ms = regexDateTime->CaptureNames().IndexOf(L"ms");

				FINALIZE_GLOBAL_STORAGE_CLASS
					delete regexDateTime;
					regexDateTime = nullptr;

			END_GLOBAL_STORAGE_CLASS(DateTimeSerializerStorage)

			DateTime TypedValueSerializerProvider<DateTime>::GetDefaultValue()
			{
				return DateTime();
			}

			WString FormatDigits(vint number, vint length)
			{
				WString result = itow(number);
				while (result.Length() < length)
				{
					result = L"0" + result;
				}
				return result;
			}

			bool TypedValueSerializerProvider<DateTime>::Serialize(const DateTime& input, WString& output)
			{
				output =
					FormatDigits(input.year, 4) + L"-" + FormatDigits(input.month, 2) + L"-" + FormatDigits(input.day, 2) + L" " +
					FormatDigits(input.hour, 2) + L":" + FormatDigits(input.minute, 2) + L":" + FormatDigits(input.second, 2) + L"." +
					FormatDigits(input.milliseconds, 3);
				return true;
			}

			bool TypedValueSerializerProvider<DateTime>::Deserialize(const WString& input, DateTime& output)
			{
				auto& dts = GetDateTimeSerializerStorage();
				Ptr<RegexMatch> match = dts.regexDateTime->Match(input);
				if (!match) return false;
				if (!match->Success()) return false;
				if (match->Result().Start() != 0) return false;
				if (match->Result().Length() != input.Length()) return false;

				vint year = wtoi(match->Groups()[dts._Y].Get(0).Value());
				vint month = wtoi(match->Groups()[dts._M].Get(0).Value());
				vint day = wtoi(match->Groups()[dts._D].Get(0).Value());
				vint hour = wtoi(match->Groups()[dts._h].Get(0).Value());
				vint minute = wtoi(match->Groups()[dts._m].Get(0).Value());
				vint second = wtoi(match->Groups()[dts._s].Get(0).Value());
				vint milliseconds = wtoi(match->Groups()[dts._ms].Get(0).Value());

				output = DateTime::FromDateTime(year, month, day, hour, minute, second, milliseconds);
				return true;
			}
		}
	}
}
