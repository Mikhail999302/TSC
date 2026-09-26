#include "MathFactor.h"
#include "MathExpressions.h"
#include "MathFunctions.h"

using namespace MathCalc;

CMathFactor::CMathFactor(MathFactorType FactorType /*= ftNumber*/, 
						 bool bCreateItem /*= false*/, 
						 int _Degree /*= 1*/): Degree(_Degree), Type(FactorType)
{
	//Note: Type is set even if not should be created
	//Type = FactorType;
	pNumber = NULL;
	pParameter = NULL;
	pExpression = NULL;
	pFunction = NULL;

	if (bCreateItem) 
	{
		switch (Type) 
		{
		case ftNumber:
			pNumber = new CNumber;
			break;
		case ftParameter:
			pParameter = new CMathParameter;
			break;
		case ftExpression:
			pExpression = new CMathExpression(ctNumber);
			break;
		case ftFunction:
			pFunction = NULL; // WARNING! bCreateItem is ignored here
			break;
		}
	}
}

CMathFactor::CMathFactor( const CNumber& Number ): 
	Degree(1), Type(ftNumber), 
	pParameter(0), pExpression(0), pFunction(0)
{
	pNumber = new CNumber(Number);
}

CMathFactor::CMathFactor( const CMathParameter& Parameter, int _Degree ): 
	Degree(_Degree), Type(ftParameter),
	pNumber(0), pExpression(0), pFunction(0)
{
	pParameter = new CMathParameter(Parameter);
}

CMathFactor::CMathFactor( const CMathExpression& Expression, int _Degree ): 
	Degree(_Degree), Type(ftExpression),
	pNumber(0), pParameter(0), pFunction(0)
{
	pExpression = new CMathExpression(Expression);
}

CMathFactor::CMathFactor( const CMathFunction& Function, int _Degree ): 
	Degree(_Degree), Type(ftFunction),
	pNumber(0), pParameter(0), pExpression(0)
{
	pFunction = MathFunctionsStorage::CreateCopy( Function );
}

CMathFactor::~CMathFactor()
{
	DestroyData();
}

/*inline*/ void CMathFactor::DestroyData()
{
	switch(Type)
	{
	case ftNumber:
		if ( pNumber != NULL ) 
			delete pNumber;
		break;
	case ftParameter:
		if ( pParameter != NULL ) 
			delete pParameter;
		break;
	case ftExpression:
		if ( pExpression != NULL )
			delete pExpression;
		break;
	case ftFunction:
		if ( pFunction != NULL ) 
			delete pFunction;
		break;
	}
	pNumber = NULL;
	pParameter = NULL;
	pExpression = NULL;
	pFunction = NULL;
}

/*inline*/ void CMathFactor::CopyData( const CMathFactor& src )
{
	//Factor should be already cleared
	Degree = src.Degree;
	pNumber = NULL;
	pParameter = NULL;
	pExpression = NULL;
	pFunction = NULL;
	switch ( Type = src.Type ) 
	{
	case ftNumber:
		pNumber = src.pNumber ? new CNumber(*src.pNumber) : NULL;
		break;
	case ftParameter:
		pParameter = src.pParameter ? new CMathParameter(*src.pParameter) : NULL;
		break;
	case ftExpression:
		pExpression = src.pExpression ? new CMathExpression(*src.pExpression) : NULL;
		break;
	case ftFunction:
		pFunction = src.pFunction ? MathFunctionsStorage::CreateCopy( *src.pFunction ) : NULL;
		break;
	}
}

/*inline*/ void CMathFactor::MoveData( CMathFactor& src )
{
	Degree = src.Degree;
	switch ( Type = src.Type ) 
	{
	case ftNumber:
		pNumber = src.pNumber; 
		src.pNumber = NULL;
		break;
	case ftParameter:
		pParameter = src.pParameter; 
		src.pParameter = NULL;
		break;
	case ftExpression:
		pExpression = src.pExpression; 
		src.pExpression = NULL;
		break;
	case ftFunction:
		pFunction = src.pFunction; 
		src.pFunction = NULL;
		break;
	}
	//src.pNumber = NULL;
}

void CMathFactor::SwapData( CMathFactor& SwapObject )
{
	CMathFactor Temp(Type);
	Temp.Degree = Degree;
	switch ( Type ) 
	{
	case ftNumber:
		Temp.pNumber = pNumber; break;
	case ftParameter:
		Temp.pParameter = pParameter; break;
	case ftExpression:
		Temp.pExpression = pExpression; break;
	case ftFunction:
		Temp.pFunction = pFunction; break;
	} // now Temp is a shalow copy of [this] class
	Degree = SwapObject.Degree;
	switch ( Type = SwapObject.Type ) {
	case ftNumber:
		pNumber = SwapObject.pNumber; break;
	case ftParameter:
		pParameter = SwapObject.pParameter; break;
	case ftExpression:
		pExpression = SwapObject.pExpression; break;
	case ftFunction:
		pFunction = SwapObject.pFunction; break;
	} // now [this] is a shalow copy of SwapObject
	SwapObject.Degree = Temp.Degree;
	switch ( SwapObject.Type = Temp.Type ) {
	case ftNumber:
		SwapObject.pNumber = Temp.pNumber; break;
	case ftParameter:
		SwapObject.pParameter = Temp.pParameter; break;
	case ftExpression:
		SwapObject.pExpression = Temp.pExpression; break;
	case ftFunction:
		SwapObject.pFunction = Temp.pFunction; break;
	} // now SwapObject is a copy of Temp class
	// release Temp's pointers if any
	switch ( Temp.Type ) {
	case ftNumber:
		Temp.pNumber = NULL; break;
	case ftParameter:
		Temp.pParameter = NULL; break;
	case ftExpression:
		Temp.pExpression = NULL; break;
	case ftFunction:
		Temp.pFunction = NULL; break;
	}
}

bool CMathFactor::IsSimilar( const CMathFactor& Operand ) const
{
	if ( Type != Operand.Type )
		return false;

	switch ( Type ) 
	{
	case ftNumber:
		return true;
	case ftParameter:
		return pParameter->IsEqual(*Operand.pParameter);
	case ftExpression:
		return pExpression->IsEqual(*Operand.pExpression);
	case ftFunction:
		return pFunction->IsEqual(*Operand.pFunction);
	}
	return false;
}

bool CMathFactor::IsEqual( const CMathFactor& Operand ) const
{
	if ( Type != Operand.Type || Degree != Operand.Degree )
		return false;

	switch ( Type ) {
	case ftNumber:
		return *pNumber == *Operand.pNumber;
	case ftParameter:
		return pParameter->IsEqual(*Operand.pParameter);
	case ftExpression:
		return pExpression->IsEqual(*Operand.pExpression);
	case ftFunction:
		return pFunction->IsEqual(*Operand.pFunction);
	}
	return false;
}

CNumber CMathFactor::Eval() const
{
	CNumber result;
	switch ( Type ) {
	case ftNumber:
		result = *pNumber;
		break;
	case ftParameter:
		result = pParameter->Eval();
		break;
	case ftExpression:
		result = pExpression->Eval();
		break;
	case ftFunction:
		result = pFunction->Eval();
		break;
	}
	result.Power( Degree );
	return result;
}

void CMathFactor::SimplifyStruct()
{
	if ( Type == ftFunction ) 
	{
		if ( Degree == 0 ) 
		{
			delete pFunction;
			pFunction = NULL;
			Type = ftNumber;
			pNumber = new CNumber;
		}
		else 
		{
			if ( CMathExpression* pSimplifiedContent = pFunction->SimplifyStruct() ) 
			{
				delete pFunction;
				pFunction = NULL;
				Type = ftExpression;
				pExpression = pSimplifiedContent;
			}
		}
	}
	if ( Type == ftExpression ) 
	{
		if ( Degree == 0 ) 
		{
			delete pExpression;
			pExpression = NULL;
			Type = ftNumber;
			pNumber = new CNumber;
		}
		else 
		{
			// Factor->E^m->Number => Factor->Number^m
			if ( pExpression->IsNumber() ) 
			{
				pNumber = ReleasePointerAndDeleteOwner( pExpression->pNumber, pExpression );
				Type = ftNumber;
			}
			// Factor->E^m->T->F => Factor --> F^m
			else if ( pExpression->IsElement() && pExpression->pElement->IsElement() ) 
			{
				int OldDegree = Degree;
				//2DO: Check that pExpression->pElement is cleared.
				CMathFactor* pSubFactor = ReleasePointerAndDeleteOwner( pExpression->pElement->pElement, pExpression );
				MoveData( *pSubFactor );
				delete pSubFactor;
				Degree *= OldDegree;
			}
		}
	}
	else if ( Type == ftParameter ) 
	{
		if ( Degree == 0 ) 
		{
			delete pParameter;
			pParameter = NULL;
			Type = ftNumber;
			pNumber = new CNumber;
		}
	}
	if ( Type == ftNumber && Degree != 1 ) 
	{
		if ( *pNumber != 0 ) 
		{
			pNumber->Simplify();
			pNumber->Power(Degree);
		}
		Degree = 1;  // Degree for Number should be 1
	}
}

void CMathFactor::Diff( const CMathParameter& Parameter )
{
	if ( Degree == 0 )
		throw CMathException_TypeIntegrityError();

	switch ( Type ) 
	{
	case ftNumber:
		{
			*pNumber = 0;
			Degree = 1;
			return;
		}
	case ftParameter: 
		{
			int Number = 0;
			if ( pParameter->IsEqual(Parameter) ) 
			{
				if ( Degree == 1 )
					Number = 1;
				// param^n->n*param^(n-1)
				else 
				{
					//if same parameter calc its derivation and put instead itself.
					CMathParameter* pOldParameter = pParameter;
					Type = ftExpression;
					pExpression = new CMathExpression( new CMathTerm(ctList, true) );
					CMathFactor addFactor(new CNumber(Degree));
					pExpression->pElement->pElements->move_back(addFactor);
					CMathFactor addFactor2(pOldParameter, Degree - 1);
					pExpression->pElement->pElements->move_back( addFactor2);
					Degree = 1;
					return;
				}
			}
			delete pParameter;
			pParameter = NULL;
			Type = ftNumber;
			pNumber = new CNumber(Number);
			Degree = 1;
			return;
		}
	case ftExpression: 
		{
			if ( Degree == 1 )
				pExpression->Diff(Parameter);
			else 
			{
				CMathExpression* pOldExpression = pExpression;
				pExpression = new CMathExpression( new CMathTerm(ctList, true) );
				CMathFactor addFactor(new CNumber(Degree));
				pExpression->pElement->pElements->move_back(addFactor);
				CMathFactor addFactor2(pOldExpression, Degree - 1);
				pExpression->pElement->pElements->move_back(addFactor2);
				CMathFactor addFactor3(new CMathExpression(*pOldExpression));
				pExpression->pElement->pElements->move_back(addFactor3);
				pExpression->pElement->pElements->back().Diff(Parameter);
				Degree = 1;
				pExpression->SimplifyStruct();
			}
			return;
		}
	case ftFunction: 
		{
			if ( Degree == 1 ) 
			{
				CMathExpression* pDiffExpression = pFunction->Diff(Parameter);
				delete pFunction;
				pFunction = NULL;
				Type = ftExpression;
				pExpression = pDiffExpression;
			}
			else 
			{
				CMathFunction* pOldFunction = pFunction;
				Type = ftExpression;
				pExpression = new CMathExpression( new CMathTerm(ctList, true) );
				CMathFactor addFactor(new CNumber(Degree));
				pExpression->pElement->pElements->move_back(addFactor);
				CMathFactor addFactor2(MathFunctionsStorage::CreateCopy( *pOldFunction ), Degree - 1);
				pExpression->pElement->pElements->move_back(addFactor2);
				CMathFactor addFactor3(pOldFunction->Diff(Parameter));
				pExpression->pElement->pElements->move_back(addFactor3);
				delete pOldFunction;
				Degree = 1;
				pExpression->SimplifyStruct();
			}
			return;
		}
	}
}

//inline void CMathFactor::Power( int nDegree )
//{
//	Type == ftNumber ? pNumber->Power( nDegree ) : Degree *= nDegree;
//}

void CMathFactor::Serialize( CMathSerializer& ar ) 
{
	if ( ar.IsStoring() ) {
		ar << Degree;
		WriteAs_int( ar, Type );
		switch ( Type ) {
		case ftNumber:
			pNumber->Serialize( ar );
			break;
		case ftParameter:
			pParameter->Serialize( ar );
			break;
		case ftExpression:
			pExpression->Serialize( ar );
			break;
		case ftFunction:
			ar << pFunction->Index;
			pFunction->Serialize( ar );
			break;
		}
	}
	else {
		ar >> Degree;
		ReadAs_int( ar, Type );
		pNumber = NULL;
		pParameter = NULL;
		pExpression = NULL;
		pFunction = NULL;
		switch ( Type ) {
		case ftNumber:
			pNumber = new CNumber;
			pNumber->Serialize( ar );
			break;
		case ftParameter:
			pParameter = new CMathParameter;
			pParameter->Serialize( ar );
			break;
		case ftExpression:
			pExpression = new CMathExpression(ctNumber);
			pExpression->Serialize( ar );
			break;
		case ftFunction:
			int FunctionIndex;
			ar >> FunctionIndex;
			pFunction = MathFunctionsStorage::CreateFunction( FunctionIndex );
			if ( pFunction )
				pFunction->Serialize( ar );
			break;
		}
	}
}
