#ifndef NUMBER_H
#define NUMBER_H
#include "Expressions.h"
namespace MathCalc 
{
enum CMathOperationType { otAdd, otSubtract, otMultiply, otDivide };

double ReductCoefficients( double* pCoefficients, int nCount ); // returns NOD
void ReductFraction( double& a, double& b );

struct CNumber {
	double Nominator;
	double Denominator;

	CNumber( double Nom = 0, double Denom = 1 ): Nominator(Nom), Denominator(Denom) {};
	double Eval() const { return Nominator/Denominator; }
	MathString ToString( int nOutputPrecision ) const;
	CNumber& operator*= ( const CNumber& );
	CNumber& operator*= ( const double& );
	CNumber& operator/= ( const CNumber& );
	CNumber& operator/= ( const double& );
	CNumber& operator+= ( const CNumber& );
	CNumber& operator+= ( const double& );
	CNumber& operator-= ( const CNumber& );
	CNumber& operator-= ( const double& );
	bool operator==( const CNumber& Operand) const
				{return Nominator == Operand.Nominator && Denominator == Operand.Denominator;}
	bool operator==( const double& Operand) const
				{return Nominator/Denominator == Operand;}
	bool operator!=( const double& Operand) const
				{return Nominator/Denominator != Operand;}
	void Invert();
	void Power( int nDegree );
	void Simplify();
	CNumber& DoOperation( const CNumber& Operand, CMathOperationType Operation );
	void Serialize( CMathSerializer& ar );
private:
	void MinimizeExponent();
};

CNumber operator+ ( const CNumber&, const CNumber& );
CNumber operator* ( const CNumber&, const CNumber& );
}//namespace MathCalc
#endif //NUMBER_H
