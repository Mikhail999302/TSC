#include "Number.h"
#include "MathSerialization.h"

using namespace MathCalc;

void CNumber::MinimizeExponent()
{
	if ( Nominator == 0 ) 
	{
		Denominator = 1;
	}
	else 
	{ //if ( fabs(Nominator) != DBL_MAX && fabs(Denominator) != DBL_MAX ) {
		while ( (fabs(Nominator) < 1e-100 && fabs(Denominator) < 1e+100) ||
			(fabs(Denominator) < 1e-100 && fabs(Nominator) < 1e+100) ) 
		{
			Nominator *= 1e+100;
			Denominator *= 1e+100;
		}
		while ( (fabs(Nominator) > 1e+100 && fabs(Denominator) > 1e-100) ||
			(fabs(Denominator) > 1e+100 && fabs(Nominator) > 1e-100) ) 
		{
			Nominator *= 1e-100;
			Denominator *= 1e-100;
		}
	}
}

inline void CNumber::Invert() 
{
	if ( Nominator == 0 )
		throw CMathException_DivisionByZero();
	std::swap( Nominator, Denominator );
}

void CNumber::Power( int nDegree )
{
	if ( nDegree == 1 )
		return;
	if ( nDegree == 0 ) {
		Nominator = 1; 
		Denominator = 1;
		return;	
	}
	if ( nDegree < 0 ) {
		Invert();
		nDegree = -nDegree;
	}
	CNumber ThisNumber(*this);
	for (int i = 1; i < nDegree; i++) {
		Nominator *= ThisNumber.Nominator;
		Denominator *= ThisNumber.Denominator;
	}
	MinimizeExponent();
}

void CNumber::Simplify()
{
	double Value = Nominator / Denominator;
	if ( floor(Value) == Value && Value * Denominator == Nominator ) {
		Nominator = Value;
		Denominator = 1;
	}
	else {
		MinimizeExponent();
		ReductFraction( Nominator, Denominator );
	}
}

CNumber& CNumber::DoOperation( const CNumber& Operand, CMathOperationType Operation )
{
	switch (Operation) {
	case otAdd:
		*this += Operand; break;
	case otMultiply:
		*this *= Operand; break;
	case otSubtract:
		*this -= Operand; break;
	case otDivide:
		*this /= Operand; break;
	}
	Simplify();
	return *this;
}

void CNumber::Serialize( CMathSerializer& ar ) 
{
	if ( ar.IsStoring() )
		ar << Nominator << Denominator;
	else
		ar >> Nominator >> Denominator;
}

/*inline*/ CNumber& CNumber::operator*= ( const CNumber& Operand ) {
	Nominator *= Operand.Nominator;
	Denominator *= Operand.Denominator;
	MinimizeExponent();
	return *this;
}
inline CNumber& CNumber::operator*= ( const double& Operand ) 
{
	Nominator *= Operand;
	MinimizeExponent();
	return *this;
}
/*inline*/ CNumber& CNumber::operator/= ( const CNumber& Operand ) 
{
	Nominator *= Operand.Denominator;
	Denominator *= Operand.Nominator;
	MinimizeExponent();
	return *this;
}
inline CNumber& CNumber::operator/= ( const double& Operand ) 
{
	Denominator *= Operand;
	MinimizeExponent();
	return *this;
}
/*inline*/ CNumber& CNumber::operator+= ( const CNumber& Operand ) 
{
	Nominator = Nominator * Operand.Denominator + Operand.Nominator * Denominator;
	Denominator *= Operand.Denominator;
	MinimizeExponent();
	return *this;
}
inline CNumber& CNumber::operator+= ( const double& Operand ) 
{
	Nominator += Operand * Denominator;
	MinimizeExponent();
	return *this;
}
inline CNumber& CNumber::operator-= ( const CNumber& Operand ) 
{
	Nominator = Nominator * Operand.Denominator - Operand.Nominator * Denominator;
	Denominator *= Operand.Denominator;
	MinimizeExponent();
	return *this;
}
inline CNumber& CNumber::operator-= ( const double& Operand ) 
{
	Nominator -= Operand * Denominator;
	MinimizeExponent();
	return *this;
}

inline CNumber operator+ ( const CNumber& Operand1, const CNumber& Operand2 ) 
{
	CNumber result(Operand1);
	result += Operand2;
	return result;
}

double MathCalc::ReductCoefficients( double* pCoefficients, int nCount )
{
	for ( int i = 0; i < nCount; i++ )
		if ( floor(pCoefficients[i]) != pCoefficients[i] ) // by this moment it works only with integers
			return 1;

	double Minimum = pCoefficients[0];
	for (int i = 0; i < nCount; i++ )
		if ( Minimum > pCoefficients[i] )
			Minimum = pCoefficients[i];

	bool IsNOD;
	for ( double NOD = Minimum; NOD > 1; --NOD ) {
		IsNOD = true;
		for (int i = 0; i < nCount; i++ ) {
			if ( ((__int64)pCoefficients[i] % (__int64)NOD) != 0 ) {
				IsNOD = false;
				break;
			}
		}
		if ( IsNOD ) {
			for (int i = 0; i < nCount; i++ )
				pCoefficients[i] = floor(pCoefficients[i] / NOD);
			return NOD;
		}
	}

	return 1;
}

void MathCalc::ReductFraction( double& a, double& b )
{
	double ab[2];
	ab[0] = a;
	ab[1] = b;
	ReductCoefficients(ab, 2);
	a = ab[0];
	b = ab[1];
}
