// MathSerialization.cpp: MathCalc classes serialization 
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MathCalcMisc.h"
#include "MathSerialization.h"

/*#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/

void MathCalc::CMathSerializer::WriteCount( DWORD dwCount )
{
	if (dwCount < 0xFFFF)
		*this << (WORD)dwCount;
	else {
		*this << (WORD)0xFFFF;
		*this << dwCount;
	}
}

MathCalc::DWORD MathCalc::CMathSerializer::ReadCount()
{
	WORD wCount;
	*this >> wCount;
	if (wCount != 0xFFFF)
		return wCount;

	DWORD dwCount;
	*this >> dwCount;
	return dwCount;
}

void MathCalc::CMathSerializer::ReadString( MathString& str )
{
	str.resize( ReadCount() );
	for ( MathString::iterator it = str.begin(); it != str.end(); ++it )
		*this >> *it;
}

void MathCalc::CMathSerializer::WriteString( const MathString& str )
{
	WriteCount( str.size() );
	for ( MathString::const_iterator it = str.begin(); it != str.end(); ++it )
		*this << *it;
}
