#ifndef FORMULA_HEADERS_MATHEXPSTRING_HPP
#define FORMULA_HEADERS_MATHEXPSTRING_HPP
#include "..\Headers\MathExpString.h"

template<typename String> Tsc::Formula::CMathExpString<String>& Tsc::Formula::CMathExpString<String>::operator+=(const CMathExpString &_rhs)
{
	mStr+="+";
	mStr+=_rhs.mStr;
	return *this;
}
#endif //FORMULA_HEADERS_MATHEXPSTRING_HPP
