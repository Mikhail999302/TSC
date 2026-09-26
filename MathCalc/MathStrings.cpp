// MathStrings.cpp: implementation of the string functions for CMath... 
// classes
//
//////////////////////////////////////////////////////////////////////
//#include "..\Dumper\Dump.h"

//#include "../Headers/stdafx.h"
#include "MathCalcMisc.h"
#include "MathExpressions.h"
#include "MathFunctions.h"
#include "MathStrings.h"

/*#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/

using namespace MathCalc;

//////////////////////////////////////////////////////////////////////

bool CExpression::ParseString( const MathString& pszExpression )
{
	CMathStringParser parser;
	return parser.ParseString( pszExpression, *m_pExpression );
}

//////////////////////////////////////////////////////////////////////
// CMathStringParser Class
// String analysis for extraction of math expression 
//////////////////////////////////////////////////////////////////////

class CMathParsingException : public CMathException {};

bool CMathStringParser::ParseString( const MathString& pszExpression, CMathExpression& result )
{
	MathString _expr(pszExpression);
	MathStrings::TrimAllSpaces( _expr );
	int length = (int)_expr.size();
	// check expression for correct symbols
	for ( int i = 0; i < length; i++ )
		if ( !MathStrings::IsDelimiter(_expr[i]) && !MathStrings::IsDigit(_expr[i]) &&
			 !MathStrings::IsAlpha(_expr[i]) && _expr[i] != TCHAR('.') )
			return false;
	
	expr = _expr.c_str();
	bool retval;
	MathFunctionsStorage::Init();
	try {
		ParseStringLevel1( result );
		retval = true;
	}
	catch (CMathException&) 
	{
		std::string dumpStr("Error:Parse String\n");
		std::string filterStr("Exception");
//		Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
		result = 0;
		retval = false;
	}

	return retval;
}

void CMathStringParser::ParseStringLevel1( CMathExpression& result )
{
	GetToken(); 
	ParseStringLevel2( result );
}

// Сложение или вычитание двух термов
void CMathStringParser::ParseStringLevel2( CMathExpression& result )
{
	TCHAR operation;

	ParseStringLevel3( result );
	while( (operation = *token) == TCHAR('+') || operation == TCHAR('-') ) {
		GetToken();
		CMathExpression Expression(ctNumber);
		ParseStringLevel3( Expression );
		if ( operation == TCHAR('+') )
			result.Add(Expression, true);
		else
			result.Subtract(Expression, true);
	}
}

// Вычисление произведения или частного двух факторов
void CMathStringParser::ParseStringLevel3( CMathExpression& result )
{
	TCHAR operation;

	ParseStringLevel4( result );
	while( (operation = *token) == TCHAR('*') || operation == TCHAR('/') ) {
		GetToken();
		CMathExpression Expression(ctNumber);
		ParseStringLevel4( Expression );
		if ( operation == TCHAR('*') )
			result.Multiply(Expression, true);
		else
			result.Divide(Expression, true);
	}
}

// Обработка производной выражения или степени параметра или числа (целочисленной)
void CMathStringParser::ParseStringLevel4( CMathExpression& result )
{
	TCHAR operation;

	ParseStringLevel5( result );
	while ( (operation = *token) == TCHAR('[') || *token == TCHAR('^') ) {
		GetToken();
		if ( operation == TCHAR('[') ) {
			if ( token_type != PARAMETER )
				throw CMathParsingException();
			CMathParameter parameter = GetParameterFromToken();
			double order = 1;
			if ( *token == TCHAR(',') ) {
				GetToken();
				if ( token_type != NUMBER )
					throw CMathParsingException();
				order = GetNumberFromToken();
				if ( double(int(order)) != order )
					throw CMathParsingException();
			}
			if ( *token != TCHAR(']') )
				throw CMathParsingException();
			GetToken();
			result.Diff( parameter, int(order) );
		}
		else { // operation == TCHAR('^')
			bool bNegativeDegree = false;
			bool bBracketNeeded = false;
			if ( *token == TCHAR('(') ) {
				bBracketNeeded = true;
				GetToken();
				if ( *token == TCHAR('-') ) {
					bNegativeDegree = true;
					GetToken();
				}
			}
			if ( token_type != NUMBER )
				throw CMathParsingException();
			double degree = GetNumberFromToken();
			if ( bNegativeDegree )
				degree = -degree;
			if ( floor(degree) != degree )
				throw CMathParsingException();
			result.Power( int(degree) );
			if ( bBracketNeeded ) {
				if ( *token != TCHAR(')') )
					throw CMathParsingException();
				GetToken();
			}
		}
	}
}

// Унарный + или -
void CMathStringParser::ParseStringLevel5( CMathExpression& result )
{
	TCHAR operation='*';

	if ( *token == TCHAR('+') || *token == TCHAR('-') ) {
		operation = *token;
		GetToken();
	}
	ParseStringLevel6( result );
	if ( operation == TCHAR('-') )
		result.InvertSign();
}

// Обработка выражения в круглых скобках
void CMathStringParser::ParseStringLevel6( CMathExpression& result )
{
	if( *token == TCHAR('(') ) {
		ParseStringLevel1( result );
		if( *token != TCHAR(')') )
			throw CMathParsingException();
		GetToken();
	}
	else {
		switch ( token_type ) {
		case NUMBER:
			result = CMathExpression( GetNumberFromToken() );
			break;
		case PARAMETER:
			result = CMathExpression( GetParameterFromToken() );
			break;
		case FUNCTION: {
			result = CMathExpression( GetFunctionFromToken(), true );
			ParseStringLevel1( *result.pElement->pElement->pFunction->pExpression );
			result.SimplifyStruct();
			/*std::list CMathExpression
				while ( *token == TCHAR(',') ) {
					GetToken();
					
				}*/
			if ( *token != TCHAR(')') )
				throw CMathParsingException();
			GetToken();
			break;
		}
		default:
			throw CMathParsingException();
		}
	}
}

double CMathStringParser::GetNumberFromToken()
{
	if ( token_type == NUMBER ) {
		TCHAR* end_unused_pos;
		double result = _tcstod( token, &end_unused_pos );
		GetToken();
		return result;
	}
	else
		throw CMathParsingException();
}

CMathParameter CMathStringParser::GetParameterFromToken()
{
	if ( token_type == PARAMETER ) {
		CMathParameter result( token );
		GetToken();
		return result;
	}
	else
		throw CMathParsingException();
}

CMathFunction* CMathStringParser::GetFunctionFromToken()
{
	if ( token_type == FUNCTION ) {
		CMathFunction* presult = MathFunctionsStorage::CreateFunction(token, true);
		if ( !presult )
			throw CMathParsingException();
		GetToken();
		return presult;
	}
	else
		throw CMathParsingException();
}

CMathStringParser::TokenType CMathStringParser::GetToken()
{
	TCHAR* temp = token;

	if( *expr == 0 )  {
		*token = 0;
		return token_type = DELIMITER;
	}

	if( MathStrings::IsDelimiter(*expr) ) { // разделитель
		token[0] = *expr;
		token[1] = 0;
		expr++; // переход на слкдующую позицию
		return token_type = DELIMITER;
	}

	if( MathStrings::IsDigit(*expr) ) { // число
		do *temp++ = *expr++;
		while( *expr != 0 && (MathStrings::IsDigit(*expr) || *expr == TCHAR('.')) );
		*temp = 0;
		return token_type = NUMBER;
	}

	if( MathStrings::IsAlpha(*expr) )  { // переменная или функция
		do *temp++=*expr++; 
		while( *expr != 0 && (MathStrings::IsAlpha(*expr) || MathStrings::IsDigit(*expr)) );
		*temp = 0;
		if ( *expr == TCHAR('(') )
			return token_type = FUNCTION;
		else
			return token_type = PARAMETER;
	}
	
	throw CMathParsingException();
}
