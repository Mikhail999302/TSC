#ifndef MATHEXPRESSIONS_H
#define MATHEXPRESSIONS_H

// MathExpressions.h: implementation of the CMathExpression class.
//
//////////////////////////////////////////////////////////////////////

#include "Expressions.h"

//-->Sergey
#include "Number.h"
//<--Sergey

#include "MathTemplates.h"

//-->Sergey
#include "MathParameter.h"
#include "MathFactor.h"
#include "MathTerm.h"
//<--Sergey

namespace MathCalc {

class CMathExpression : public CMathContainerBase< CMathTerm >
{
public:
	CMathExpression( double Number = 0 );
	CMathExpression( const CMathExpression& src ): 
		CMathContainerBase<CMathTerm>( ctNumber, false ) { CopyData( src ); }
	explicit CMathExpression( MathContainerType ContainerType, bool bCreateItem = false ): 
		CMathContainerBase<CMathTerm> (ContainerType, bCreateItem) {};
	explicit CMathExpression( const CNumber& Number ): CMathContainerBase<CMathTerm>(Number) {};
	explicit CMathExpression( const CMathTerm& Term ): CMathContainerBase<CMathTerm>(Term) {};
	explicit CMathExpression( const CMathParameter& Parameter );
	explicit CMathExpression( CMathTerm* pTerm ): CMathContainerBase<CMathTerm>(pTerm) {}
	explicit CMathExpression( CMathFunction* pFunction, bool bOwnMode );
	CMathExpression& operator=( const CMathExpression& src );

// Operations
	MathString ToString( int nOutputPrecision = 2 ) const;
	CNumber Eval() const;

	void CollectSimilarTerms( ElementsIterator SecondListStart );
	void CollectSimilarTerms() { CollectSimilarTerms( pElements->end() ); }
	void SimplifyStruct();

// Arithmetic operations
	void Power( int Degree );
	void InvertSign();
	void Diff( const CMathParameter& Parameter );
	void Diff( const CMathParameter& Parameter, int Order ) 
	{
		for (int i = 0; i < Order; i++)
		{
			Diff( Parameter );
		}
	}
	//-->Sergey
	//Const added
	CMathExpression& Add		(const CMathExpression&, bool bOwnMode = false );
	CMathExpression& Subtract	(const CMathExpression&, bool bOwnMode = false );
	//<--Sergey
	CMathExpression& Multiply	(CMathExpression&, bool bOwnMode = false );
	CMathExpression& Divide		(CMathExpression&, bool bOwnMode = false );
};

} // namespace MathCalc
#endif //MATHEXPRESSIONS_H
