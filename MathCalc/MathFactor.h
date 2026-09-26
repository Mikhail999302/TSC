#ifndef MATHFACTOR_H
#define MATHFACTOR_H

#include "MathCalcDebug.h"

#include "Number.h"
#include "MathParameter.h"
#include "MathTemplates.h"

namespace MathCalc 
{
class CMathExpression;
class CMathParameter;
class CMathFunction;

enum MathFactorType { ftNumber = 0, ftParameter, ftExpression, ftFunction };

class CMathFactor
{
public:
	int Degree ; // != 0
	MathFactorType Type;
	//union 
	//{
		CNumber* pNumber;
		CMathParameter* pParameter;
		CMathExpression* pExpression;
		CMathFunction* pFunction;
	//};
//public:

	CMathFactor( const CMathFactor& src ) { CopyData( src ); }
	explicit CMathFactor( MathFactorType FactorType = ftNumber, bool bCreateItem = false, int _Degree = 1 );
	explicit CMathFactor( const CNumber& Number );
	explicit CMathFactor( const CMathParameter& Parameter, int _Degree = 1 );
	explicit CMathFactor( const CMathExpression& Expression, int _Degree = 1 );
	explicit CMathFactor( const CMathFunction& Function, int _Degree = 1 );
	explicit CMathFactor( CNumber* ptr, int deg = 1 ): 
				Degree(deg),Type(ftNumber), pNumber(ptr) ,
				pParameter(0), pExpression(0), pFunction(0)
				{}
	explicit CMathFactor( CMathParameter* ptr, int deg = 1 ): 
				Degree(deg), Type(ftParameter), pNumber(0),
				pParameter(ptr), pExpression(0), pFunction(0)
				{}
	explicit CMathFactor( CMathExpression* ptr, int deg = 1 ): 
				Degree(deg), Type(ftExpression), pNumber(0),
				pParameter(0), pExpression(ptr), pFunction(0)
				{}
	explicit CMathFactor( CMathFunction* ptr, int deg = 1 ): 
				Degree(deg), Type(ftFunction), pNumber(0),
				pParameter(0), pExpression(0), pFunction(ptr) 
				{}
	~CMathFactor();
	CMathFactor& operator=( const CMathFactor& src )
	{
		if ( this != &src ) 
		{
			DestroyData();
			CopyData( src );
		}
		return *this;
	}

	// Operations
	bool IsNumber() const { return Type == ftNumber; }
	bool IsParameter() const { return Type == ftParameter; }
	bool IsExpression() const { return Type == ftExpression; }
	bool IsFunction() const { return Type == ftFunction; }

	MathString ToString( int nOutputPrecision ) const;
	CNumber Eval() const;
	bool IsSimilar( const CMathFactor& Operand ) const;
	bool IsEqual( const CMathFactor& Operand ) const;

	void Diff( const CMathParameter& Parameter );
	void SimplifyStruct();
	void Power( int nDegree )
	{
		if(Type == ftNumber)
			pNumber->Power( nDegree );
		else 
			Degree *= nDegree;
	}
	void Serialize( CMathSerializer& ar );

	void SwapData( CMathFactor& SwapObject );
	void MoveData( CMathFactor& );
private:
	void CopyData( const CMathFactor& );
	void DestroyData();
};

typedef CMathBasesList<CMathFactor> CMathFactorsList;

}//namespace MathCalc
#endif //MATHFACTOR_H
