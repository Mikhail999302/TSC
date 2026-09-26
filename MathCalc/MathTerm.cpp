#include "MathTerm.h"
#include "MathExpressions.h"
using namespace MathCalc;

inline CMathTerm& CMathTerm::operator=( const CMathTerm& src )
{
	if ( this != &src ) {
		DestroyData();
		CopyData( src );
	}
	return *this;
}

CNumber CMathTerm::Eval() const
{
	if ( Type == ctList ) {
		CNumber result(1);
		for ( ElementsIterator it = pElements->begin(); it != pElements->end(); ++it )
			result *= it->Eval();
		return result;
	}
	else
		return BaseContainerClass::BaseEval();
}

//inline CNumber* CMathTerm::GetMultiplier()
//{
//	switch ( Type ) {
//	case ctNumber:
//		return pNumber;
//	case ctList:
//		return ( pElements->front().IsNumber() ? pElements->front().pNumber : NULL );
//	default:
//		return NULL;
//	}
//}

//inline void CMathTerm::Multiplier_Add( const CNumber& Operand ) {
//	DoMultiplierOperation( Operand, otAdd );
//}

//inline void CMathTerm::Multiplier_Multiply( const CNumber& Operand ) {
//	DoMultiplierOperation( Operand, otMultiply );
//}

//inline void CMathTerm::Multiplier_Subtract( const CNumber& Operand ) {
//	DoMultiplierOperation( Operand, otSubtract );
//}

//inline void CMathTerm::Multiplier_Divide( const CNumber& Operand ) {
//	DoMultiplierOperation( Operand, otDivide );
//}

void CMathTerm::DoMultiplierOperation( const CNumber& Operand, CMathOperationType Operation )
{
	switch ( Type ) {
	case ctNumber:
		pNumber->DoOperation(Operand, Operation);
		break;
	case ctElement:
		ConvertToList();
		pElements->push_front( CMathFactor(CNumber(1).DoOperation(Operand, Operation)) );
		break;
	case ctList:
		if ( pElements->front().IsNumber() )
			pElements->front().pNumber->DoOperation(Operand, Operation);
		else
			pElements->push_front( CMathFactor(CNumber(1).DoOperation(Operand, Operation)) );
		break;
	}
}

void CMathTerm::SimplifyMultiplier()
{
	if ( Type == ctList && pElements->front().IsNumber() && *pElements->front().pNumber == 1 ) {
		pElements->erase( pElements->begin() );
		TryShrinkList();
	}
}

bool CMathTerm::IsSimilar( const CMathTerm& Operand ) const
{
	switch ( Type ) {
	case ctNumber: // number1 ~ number2
		return Operand.Type == ctNumber;
	case ctElement: // x ~ x || x ~ number * x
		return ( Operand.IsElement() && pElement->IsEqual(*Operand.pElement) ) ||
			( Operand.IsList() && Operand.pElements->size() == 2 && 
			Operand.pElements->front().IsNumber() && pElement->IsEqual(Operand.pElements->back()) );
	case ctList: // element ~ list? [number1] * list1 ~ [number2] * list2
		switch ( Operand.Type ) {
	case ctNumber:
		return false;
	case ctElement:
		return Operand.IsSimilar( *this );
	case ctList: {
		if ( pElements->size() < 2 || Operand.pElements->size() < 2 )
			throw CMathException_TypeIntegrityError();

		int NumsCount = 0, OperandNumsCount = 0;
		ElementsIterator it = pElements->begin();
		if ( pElements->front().IsNumber() ) {
			++it;
			NumsCount = 1;
		}
		ElementsIterator Operand_it = Operand.pElements->begin();
		if ( Operand.pElements->front().IsNumber() ) {
			++Operand_it;
			OperandNumsCount = 1;
		}
		if ( pElements->size() - NumsCount != Operand.pElements->size() - OperandNumsCount )
			return false;

		for ( ; it != pElements->end(); ++it ) {
			if ( Operand.pElements->FindElement( *it, Operand_it ) == Operand.pElements->end() )
				return false;
		}
		return true;
				 }
		}
	}
	return false;
}

void CMathTerm::CollectSimilarFactors()
{
	if ( Type != ctList )
		throw CMathException_TypeIntegrityError();

	// Collect numbers and collect similar factors
	CNumber Multiplier(1);
	for ( ElementsIterator it = pElements->begin(); it != pElements->end(); ) {
		if ( it->IsNumber() ) {
			if ( it->Degree != 1 )
				throw CMathException_TypeIntegrityError();
			Multiplier *= *it->pNumber;
			it = pElements->erase( it );
		}
		else { // look for similar factors from the list's tail
			ElementsIterator sub_it = it;
			++sub_it;
			while ( sub_it != pElements->end() ) {
				if ( it->IsSimilar( *sub_it ) ) {
					it->Degree += sub_it->Degree;
					sub_it = pElements->erase( sub_it );
				}
				else
					++sub_it;
			}
			++it;
		}
	}
	// Remove all factors with 0 degree
	for (ElementsIterator it = pElements->begin(); it != pElements->end(); ) {
		if ( it->Degree == 0 )
			it = pElements->erase( it );
		else
			++it;
	}
	// Put result multiplier != 1 to the head of the list
	Multiplier.Simplify();
	if ( Multiplier == 0 || pElements->size() == 0 ) {
		delete pElements;
		Type = ctNumber;
		pNumber = new CNumber(Multiplier);
	}
	else if ( Multiplier != 1 )
		pElements->push_front( CMathFactor(Multiplier) );
	else
		TryShrinkList();
}

void CMathTerm::SimplifyStruct()
{
	if ( Type == ctElement ) {
		pElement->SimplifyStruct();
		if ( pElement->IsNumber() ) {
			pNumber = ReleasePointerAndDeleteOwner( pElement->pNumber, pElement );
			Type = ctNumber;
		}
		// Term->F->E^m->T->(F1,...,Fn) --> Term->(F1^m,...,Fn^m)
		if ( pElement->IsExpression() && pElement->pExpression->IsElement() ) {
			if ( pElement->Degree == 1 ) // m = 1
				ExtractDataFromSubItem( pElement->pExpression->pElement );
			else { // m != 1
				int nFactorDegree = pElement->Degree;
				ExtractDataFromSubItem( pElement->pExpression->pElement );
				for ( ElementsIterator it = pElements->begin(); it != pElements->end(); ++it )
					it->Power( nFactorDegree );
			}
		}
	}
	else if ( Type == ctList ) {
		// Chain subfactors into one list: Term->F->E^m->T->(F1,...,Fn) --> Term->(F1^m,...,Fn^m)
		for ( ElementsIterator it = pElements->begin(); it != pElements->end(); ) {
			it->SimplifyStruct();
			if ( it->IsExpression() && it->pExpression->IsElement() ) {
				if ( !it->pExpression->pElement->IsList() )
					throw CMathException_TypeIntegrityError();
				ElementsIterator sub_it = pElements->insert_(it, 
					it->pExpression->pElement->pElements->begin(), 
					it->pExpression->pElement->pElements->end(), true);
				if ( it->Degree != 1 ) { // m != 1
					for (; sub_it != it; ++sub_it )
						sub_it->Power( it->Degree );
				}
				it = pElements->erase( it );
			}
			else
				++it;
		}		
		CollectSimilarFactors();	
	}
}

void CMathTerm::Diff( const CMathParameter& Parameter )
{
	switch ( Type ) 
	{
	case ctNumber:
		*pNumber = 0;
		break;
	case ctElement:
		pElement->Diff(Parameter);
		break;
	case ctList: 
		{
			// T->{F1,...,Fn} --> T->F->E->{T1,...,Tn},  Ti: Ti->{F1,...,Fi',...,Fn}
			CMathTerm ThisTerm;
			ThisTerm.MoveData(*this);
			Type = ctElement;
			pElement = new CMathFactor( new CMathExpression(ctList, true) );
			CMathFactor DiffFactor;
			for ( ElementsIterator it = ThisTerm.pElements->begin(); it != ThisTerm.pElements->end(); ++it ) 
			{
				DiffFactor = *it;
				DiffFactor.Diff(Parameter);
				// add Ti if Fi' != 0
				//-->Sergey
				//if ( !(DiffFactor.IsNumber() && *DiffFactor.pNumber == 0) &&
				//	!(DiffFactor.IsExpression() && *DiffFactor.pExpression->pNumber == 0) ) 
				//{ 
				//	pElement->pExpression->pElements->move_back( CMathTerm(ctList, true) );
				//	CMathTerm* pTerm = &pElement->pExpression->pElements->back();
				//	ElementsIterator sub_it;
				//	for (sub_it = ThisTerm.pElements->begin(); sub_it != it; ++sub_it )
				//										pTerm->pElements->push_back( *sub_it );
				//	pTerm->pElements->move_back( DiffFactor );
				//	while ( ++sub_it != ThisTerm.pElements->end() )
				//										pTerm->pElements->push_back( *sub_it );
				//}
				if ( !(DiffFactor.IsNumber() && *DiffFactor.pNumber == 0) &&
					!(DiffFactor.IsExpression() && DiffFactor.pExpression->IsNumber()
					&& *DiffFactor.pExpression->pNumber == 0) ) 
				{
					CMathTerm addTerm(ctList,true);
					pElement->pExpression->pElements->move_back( addTerm );
					CMathTerm* pTerm = &pElement->pExpression->pElements->back();
					ElementsIterator sub_it=ThisTerm.pElements->begin();
					for (sub_it = ThisTerm.pElements->begin(); sub_it != it; ++sub_it )
						pTerm->pElements->push_back( *sub_it );
					pTerm->pElements->move_back( DiffFactor );
					while ( ++sub_it != ThisTerm.pElements->end() )
						pTerm->pElements->push_back( *sub_it );
				}
				//<--Sergey
			}
			if ( pElement->pExpression->pElements->size() == 0 ) 
			{
				delete pElement;
				pElement = NULL;
				Type = ctNumber;
				pNumber = new CNumber;
			}
			else 
			{
				pElement->pExpression->TryShrinkList();
				pElement->pExpression->SimplifyStruct();
			}
			break;
		}
	}
}
