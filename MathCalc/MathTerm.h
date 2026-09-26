#ifndef MATHTERM_H
#define MATHTERM_H

#include "Number.h"
#include "MathFactor.h"
#include "MathTemplates.h"

namespace MathCalc 
{

class CMathTerm : public CMathContainerBase< CMathFactor >
{
public:
	CMathTerm( const CMathTerm& src ): 
	  CMathContainerBase<CMathFactor>( ctNumber, false ) { CopyData( src ); }
	explicit CMathTerm( MathContainerType ContainerType = ctNumber, bool bCreateItem = false ): 
	CMathContainerBase<CMathFactor> (ContainerType, bCreateItem) {};
	explicit CMathTerm( const CNumber& Number ): CMathContainerBase<CMathFactor>(Number) {};
	explicit CMathTerm( const CMathFactor& Factor ): CMathContainerBase<CMathFactor>(Factor) {};
	explicit CMathTerm( CMathFactor* pFactor ): CMathContainerBase<CMathFactor>(pFactor) {}
	CMathTerm& operator=( const CMathTerm& src );

	// Operations
	MathString ToString( int nOutputPrecision ) const;
	CNumber Eval() const;
	bool IsSimilar( const CMathTerm& Operand ) const;

	void Diff( const CMathParameter& Parameter );
	void SimplifyStruct();
	void CollectSimilarFactors();

	CNumber* GetMultiplier()
	{
		switch ( Type ) 
		{
		case ctNumber:
			return pNumber;
		case ctList:
			return ( pElements->front().IsNumber() ? pElements->front().pNumber : NULL );
		default:
			return NULL;
		}
	}
	void SimplifyMultiplier();
	void Multiplier_Add( const CNumber& Operand )
	{
		DoMultiplierOperation( Operand, otAdd );
	}
	void Multiplier_Multiply( const CNumber& Operand )
	{
		DoMultiplierOperation( Operand, otMultiply );
	}
	void Multiplier_Subtract( const CNumber& Operand )
	{
		DoMultiplierOperation( Operand, otSubtract );
	}
	void Multiplier_Divide( const CNumber& Operand )
	{
		DoMultiplierOperation( Operand, otDivide );
	}
private:
	void DoMultiplierOperation( const CNumber& Operand, CMathOperationType Operation );
};

typedef CMathBasesList<CMathTerm> CMathTermsList;
}//namespace MathCalc
#endif //MATHTERM_H
