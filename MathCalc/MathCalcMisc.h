// MathCalcMisc.h
//////////////////////////////////////////////////////////////////////
#ifndef MATHCALCMISC_H
#define MATHCALCMISC_H

// STL includes

#include <string>
#include <vector>
#include <map>
#include <list>
#include <algorithm>

// Other includes
#include <math.h>
#include <TCHAR.h>
#include "importDefs.h"

namespace MathCalc
{
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef const TCHAR * LPCTSTR;

//////////////////////////////////////////////////////////////////////
// Exceptions used in MathCalc
// Base exception class
//->Sergey 
//Make Exceptions none exportable and inherited by exception
//#include<exception>
//<-Sergey
class MATHCALC_API CMathException
{
};//:public std::exception {};
class MATHCALC_API CMathException_DivisionByZero : public CMathException
{
};
class MATHCALC_API CMathException_FunctionCopyError : public CMathException
{
};
class MATHCALC_API CMathException_InvalidParameterIndex : public CMathException
{
};
class MATHCALC_API CMathException_TypeIntegrityError : public CMathException
{
};
class MATHCALC_API CMathException_InvalidFunctionName : public CMathException
{
};

//////////////////////////////////////////////////////////////////////
// Math strings

typedef std::basic_string<TCHAR> MathString;

// Inline helper functions
namespace MathStrings
{
MATHCALC_API bool IsAlpha( TCHAR c );
MATHCALC_API bool IsDigit( TCHAR c );
MATHCALC_API bool IsDelimiter( TCHAR c );
MATHCALC_API bool IsWhitespace( TCHAR c );
MATHCALC_API void TrimAllSpaces( MathString& str );
MATHCALC_API void Format( MathString& Target, size_t nBufSize, LPCTSTR lpszFormat, ... );
}//namespace MathStrings

//////////////////////////////////////////////////////////////////////
// Math context - information about all parameters and their values

class CMathSerializer;
namespace MathContext
{
MATHCALC_API UINT GetParametersCount();
// Adds new parameter to the context. 
// If the parameter with specified name exists then function returns it.
MATHCALC_API UINT AddParameter( const MathString& ParameterName );
// Look if specified parameter exists
MATHCALC_API bool GetParameterIndex( const MathString& ParameterName, UINT& ParameterIndex );
// Get name reference
MATHCALC_API MathString& GetParameterName( UINT ParameterIndex )
/*throw( CMathException_InvalidParameterIndex )*/;

// Operations with parameters values
MATHCALC_API double GetParameterValue( const MathString& ParameterName );
MATHCALC_API double GetParameterValue( UINT ParameterIndex );
MATHCALC_API void SetParameterValue( const MathString& ParameterName, double Value );
MATHCALC_API void SetParameterValue( UINT ParameterIndex, double Value );

// Clears the entire storage
MATHCALC_API void ResetMap();
// Clears only parameters values
MATHCALC_API void ClearValues();

// Multi-map maintainance
class CMathMap;
typedef CMathMap* MapIDType;
MATHCALC_API MapIDType AddMap();
MATHCALC_API void RemoveMap( MapIDType MapID );
MATHCALC_API void SelectMap( MapIDType MapID );

MATHCALC_API void Serialize( CMathSerializer& ar, MapIDType MapID = MapIDType(0) );
}// namespace MathContext

} // namespace MathCalc

#include "MathSerialization.h"
#endif //MATHCALCMISC_H

