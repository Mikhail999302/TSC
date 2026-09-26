// MathStringRep.cpp: string represenation for CMath... objects
// and MathStrings functions implementation
//
//////////////////////////////////////////////////////////////////////

#include <stdarg.h>

//#include "../Headers/stdafx.h"
#include "MathCalcMisc.h"
#include "MathExpressions.h"
#include "MathFunctions.h"

/*#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/

using namespace MathCalc;

//////////////////////////////////////////////////////////////////////
// MathStrings

/*inline*/ bool MathCalc::MathStrings::IsAlpha( TCHAR c )
{
	return	_istalpha( c ) || (TCHAR('_') == c);
}

/*inline*/ bool MathCalc::MathStrings::IsDigit( TCHAR c )
{
	return _istdigit( c ) != 0;
}

/*inline*/ bool MathCalc::MathStrings::IsDelimiter( TCHAR c )
{
	return (MathString(_T("+-*^/(),[]")).find( c )) != std::string::npos;
}

inline bool MathCalc::MathStrings::IsWhitespace( TCHAR c ) 
{
	return (c == TCHAR(' ') || c == TCHAR('\t') || c == TCHAR('\n') || c == TCHAR('\r'));
}

/*inline*/ void MathCalc::MathStrings::TrimAllSpaces( MathString& str )
{
	MathString::iterator end = std::remove_if( str.begin(), str.end(), IsWhitespace );
	str.erase( end, str.end() );
}

void MathCalc::MathStrings::Format( MathString& Target, size_t nBufSize, LPCTSTR lpszFormat, ... )
{
	TCHAR* pszBuffer =new TCHAR [nBufSize];
	va_list argList;
	va_start(argList, lpszFormat);
	_vsntprintf(pszBuffer, nBufSize, lpszFormat, argList);
	//_vsnprintf(pszBuffer, nBufSize,lpszFormat, argList);
	va_end(argList);
	Target = pszBuffer;
	delete[]pszBuffer;
}

//////////////////////////////////////////////////////////////////////
// Module definitions

namespace {

template<class NumberType>
void FormatNumber( MathString& Target, MathCalc::LPCTSTR lpszFormat, NumberType num )
{
	MathStrings::Format( Target, 10, lpszFormat, num );
}

MathString GetString( double Number, int nPrecision, bool bAddSign = true )
{
	if ( Number == 0 )
		return bAddSign ? _T("+0") : _T("0");
	else {
		MathString result;
		if ( floor(Number) == Number && double((__int64)Number) == Number )
			(Number > 0 && bAddSign) ? FormatNumber( result, _T("+%I64d"), (__int64)(Number) ) : FormatNumber( result, _T("%I64d"), (__int64)(Number) );
		else {
			MathString precision;
			FormatNumber( precision, "%d", nPrecision );
			MathString FormatString = (Number > 0 && bAddSign) ? _T("+%.") : _T("%.");
			FormatString += precision + _T("f");
			FormatNumber( result, FormatString.c_str(), Number );
		}
		return result;
	}
}

template< class T > 
inline MathString GetStringWithoutLeadingPlus( T& t, int nOutputPrecision )
{
	MathString result = t->ToString( nOutputPrecision );
	if ( result[0] == TCHAR('+') )
		return result.substr( 1, result.length() );
	else
		return result;
}

}

//////////////////////////////////////////////////////////////////////
// MathString representation of math objects
//////////////////////////////////////////////////////////////////////

MathString CNumber::ToString( int nOutputPrecision ) const
{
	if ( Denominator == 1 )
		return GetString(Nominator, nOutputPrecision);
	else
		return GetString(Nominator, nOutputPrecision) + _T("/") + GetString(Denominator, nOutputPrecision, false);
}

MathString CMathParameter::ToString() const
{
	return MathContext::GetParameterName(Index);
}

MathString CMathFunction::ToString( int nOutputPrecision ) const
{
	return MathFunctionsStorage::GetFunctionName(Index) + 
		   _T("(") + pExpression->ToString( nOutputPrecision ) + _T(")");
}

MathString CMathFactor::ToString( int nOutputPrecision ) const
{
	if (Degree != 1)
	{
		MathString strDegree;
		if ( Degree > 0 )
			FormatNumber( strDegree, _T("^%d"), Degree);
		else
			FormatNumber( strDegree, _T("^(%d)"), Degree);
		switch ( Type ) {
		case ftParameter:
			return pParameter->ToString() + strDegree;
		case ftExpression:
			return _T("(") + pExpression->ToString( nOutputPrecision ) + _T(")") + strDegree;
		case ftFunction:
			return _T("(") + pFunction->ToString( nOutputPrecision ) + _T(")") + strDegree;
		default: 
			throw CMathException_TypeIntegrityError();
		}
	}
	else {
		switch ( Type ) {
		case ftNumber:
			return pNumber->ToString( nOutputPrecision );
		case ftParameter:
			return pParameter->ToString();
		case ftExpression:
			return _T("(") + pExpression->ToString( nOutputPrecision ) + _T(")");
		case ftFunction:
			return pFunction->ToString( nOutputPrecision );
		}
	}
	return _T("");
}

MathString CMathTerm::ToString( int nOutputPrecision ) const
{
	switch (Type) {
	case ctNumber:
		return pNumber->ToString( nOutputPrecision );
	case ctElement:
		return _T("+") + pElement->ToString( nOutputPrecision );
	case ctList: {
		if ( pElements->size() < 2 )
			throw CMathException_TypeIntegrityError();
		MathString result;
		ElementsIterator it = pElements->begin();
		if ( !pElements->front().IsNumber() )
			result = _T("+") + it->ToString( nOutputPrecision );
		else if ( *pElements->front().pNumber == CNumber(-1) )
			result = _T("-") + (++it)->ToString( nOutputPrecision ); // skip -1
		else
			result = it->ToString( nOutputPrecision );
		for ( ++it; it != pElements->end(); ++it )
			result += _T("*") + it->ToString( nOutputPrecision );
		return result;
	}
	}
	return _T("");
}

MathString CMathExpression::ToString( int nOutputPrecision ) const
{
	switch (Type) {
	case ctNumber:
		return GetStringWithoutLeadingPlus( pNumber, nOutputPrecision );
	case ctElement:
		return GetStringWithoutLeadingPlus( pElement, nOutputPrecision );
	case ctList: {
		if ( pElements->size() < 2 )
			throw CMathException_TypeIntegrityError();
		ElementsIterator it = pElements->begin();
		MathString result = GetStringWithoutLeadingPlus( it, nOutputPrecision );
		for ( ++it; it != pElements->end(); ++it )
			result += it->ToString( nOutputPrecision );
		return result;
	}
	}
	return _T("");
}
