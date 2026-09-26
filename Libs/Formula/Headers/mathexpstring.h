#ifndef FORMULA_HEADERS_MATHEXPSTRING_H
#define FORMULA_HEADERS_MATHEXPSTRING_H
#include <iostream>

namespace Tsc {
namespace Formula {
template<typename String> class CMathExpString
{
	String mStr;
public:
	CMathExpString(const String& _str="") :
		mStr(_str)
	{
	}
	CMathExpString<String>& operator+=(const CMathExpString &_rhs);
	operator String() const
	{
		return mStr;
	}
	friend std::istream& operator>>(std::istream&_s, CMathExpString<String>&_to);
};

std::istream& operator>>(std::istream&_s, CMathExpString<std::string>&_to);
} // namespace Formula
} // namespace Tsc
#include "..\Implementation\MathExpString.hpp"
#endif //FORMULA_HEADERS_MATHEXPSTRING_H
