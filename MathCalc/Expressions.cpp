// Expressions.cpp: implementation of interface classes.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MathExpressions.h"
#include "MathFunctions.h"

/*#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/

using namespace MathCalc;

//////////////////////////////////////////////////////////////////////
// CParameter

CParameter::CParameter(void): 
	m_pParameter(new CMathParameter()) { IncreaseObjectsCounter(); }

CParameter::CParameter( const CParameter& src ): 
	m_pParameter(new CMathParameter(*src.m_pParameter)) { IncreaseObjectsCounter(); }

CParameter::CParameter( const MathString& ParameterName ): 
	m_pParameter(new CMathParameter(ParameterName)) { IncreaseObjectsCounter(); }

CParameter::~CParameter() 
{ 
	delete m_pParameter;
	DecreaseObjectsCounter();
}

CParameter& CParameter::operator=( const CParameter& src )
{
	if ( this != &src )
		*m_pParameter = *src.m_pParameter;
	return *this;
}

MathString CParameter::ToString() const
{
	return m_pParameter->ToString();
}

double CParameter::Eval() const
{
	return m_pParameter->Index >= 0 ? MathContext::GetParameterValue( m_pParameter->Index ) : 0;
}

bool CParameter::operator==( const CParameter& Operand ) const
{
	return m_pParameter->IsEqual( *Operand.m_pParameter );
}

void CParameter::SetValue( double Value ) const
{
	m_pParameter->SetValue(Value);
}

double CParameter::GetValue() const
{
	return m_pParameter->GetValue();
}

void CParameter::Serialize( CMathSerializer& ar )
{
	m_pParameter->Serialize( ar );
}

//////////////////////////////////////////////////////////////////////
// CExpression

CExpression::CExpression( double Number ): 
	m_pExpression(new CMathExpression(Number)) { IncreaseObjectsCounter(); }

CExpression::CExpression( const CExpression& src )  
{ 
	m_pExpression = new CMathExpression(*src.m_pExpression);
	IncreaseObjectsCounter(); 
}

CExpression::CExpression( const CParameter& Parameter ): 
	m_pExpression(new CMathExpression(*Parameter.m_pParameter)) { IncreaseObjectsCounter(); }

CExpression::~CExpression() 
{ 
	delete m_pExpression;
	DecreaseObjectsCounter();
}

CExpression& CExpression::operator=( const CExpression& src )
{
	if ( this != &src )
		*m_pExpression = *src.m_pExpression;
	return *this;
}

MathString CExpression::ToString( int nOutputPrecision ) const 
{ 
	return m_pExpression->ToString(nOutputPrecision); 
}

double CExpression::Eval() const 
{ 
	return m_pExpression->Eval().Eval(); 
}

void CExpression::Serialize( CMathSerializer& ar )
{
	m_pExpression->Serialize( ar );
}

bool CExpression::IsNumber() const
{
	return m_pExpression->IsNumber();
}

void CExpression::ApplyFunction( const MathString& FunctionName )
	/*throw( CMathException_InvalidFunctionName )*/
{
	CMathFunction* pFunction = MathFunctionsStorage::CreateFunction(FunctionName, false);
	if ( pFunction ) {
		pFunction->pExpression = m_pExpression;
		m_pExpression = new CMathExpression(pFunction, true);
	}
	else
		throw CMathException_InvalidFunctionName();
}

void CExpression::Power( int Degree ) 
{ 
	m_pExpression->Power(Degree); 
}

void CExpression::InvertSign() 
{ 
	m_pExpression->InvertSign(); 
}

void CExpression::Diff( const CParameter& Parameter ) 
{ 
	m_pExpression->Diff(*Parameter.m_pParameter); 
}

void CExpression::Diff( const CParameter& Parameter, int Order ) 
{ 
	m_pExpression->Diff(*Parameter.m_pParameter, Order); 
}

CExpression& CExpression::Add(const CExpression& E, bool bOwnMode ) 
{ 
	m_pExpression->Add(*E.m_pExpression, bOwnMode); 
	return *this;
}

CExpression& CExpression::Subtract(const CExpression& E, bool bOwnMode ) 
{ 
	m_pExpression->Subtract(*E.m_pExpression, bOwnMode); 
	return *this;
}

CExpression& CExpression::Multiply( CExpression& E, bool bOwnMode ) 
{ 
	m_pExpression->Multiply(*E.m_pExpression, bOwnMode); 
	return *this;
}

CExpression& CExpression::Divide( CExpression& E, bool bOwnMode ) 
{ 
	m_pExpression->Divide(*E.m_pExpression, bOwnMode); 
	return *this;
}

//////////////////////////////////////////////////////////////////////
// Operations

inline CExpression MathCalc::operator+( const CExpression& E1, const CExpression& E2 ) {
	CExpression result(E1);
	result += E2;
	return result;
}
inline CExpression MathCalc::operator-( const CExpression& E1, const CExpression& E2 ) {
	CExpression result(E1);
	result -= E2;
	return result;
}
inline CExpression MathCalc::operator*( const CExpression& E1, const CExpression& E2 ) {
	CExpression result(E1);
	result *= E2;
	return result;
}
inline CExpression MathCalc::operator/( const CExpression& E1, const CExpression& E2 ) {
	CExpression result(E1);
	result /= E2;
	return result;
}
inline CExpression MathCalc::operator^( const CExpression& E, int Degree ) {
	CExpression result(E);
	result.Power(Degree);
	return result;
}
