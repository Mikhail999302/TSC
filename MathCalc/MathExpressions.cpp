// MathExpressions.cpp: implementation of the CMath... classes.
//
//////////////////////////////////////////////////////////////////////

//#include "../Headers/stdafx.h"
#include "Number.h"
#include "MathExpressions.h"
#include "MathFunctions.h"

/*#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/

using namespace MathCalc;

//////////////////////////////////////////////////////////////////////
// CNumber
//////////////////////////////////////////////////////////////////////

inline CNumber MathCalc::operator* ( const CNumber& Operand1, const CNumber& Operand2 ) 
{
	CNumber result(Operand1);
	result *= Operand2;
	return result;
}

//////////////////////////////////////////////////////////////////////
// CMathExpression
//////////////////////////////////////////////////////////////////////

CMathExpression::CMathExpression( double Number ): 
	CMathContainerBase<CMathTerm>( ctNumber, false )
{ 
	pNumber = new CNumber(Number); 
}

CMathExpression::CMathExpression( const CMathParameter& Parameter ): 
	CMathContainerBase<CMathTerm>( ctElement, false )
{ 
	pElement = new CMathTerm( new CMathFactor( new CMathParameter(Parameter) ) ); 
}

CMathExpression::CMathExpression( CMathFunction* pFunction, bool bOwnMode ):
	CMathContainerBase<CMathTerm>( ctElement, false )
{
	pElement = new CMathTerm( new CMathFactor( bOwnMode ? pFunction : MathFunctionsStorage::CreateCopy(*pFunction) ) );
	if ( pElement->pElement->pFunction ) 
		SimplifyStruct();
}

CMathExpression& CMathExpression::operator=( const CMathExpression& src )
{
	if ( this != &src ) {
		DestroyData();
		CopyData( src );
	}
	return *this;
}

CNumber CMathExpression::Eval() const
{
	if ( Type == ctList ) {
		CNumber result;
		for ( ElementsIterator it = pElements->begin(); it != pElements->end(); ++it )
			result += it->Eval();
		return result;
	}
	else
		return BaseContainerClass::BaseEval();
}

void CMathExpression::CollectSimilarTerms( ElementsIterator SecondListStart )
{
	if ( Type != ctList )
		throw CMathException_TypeIntegrityError();

// Collect numbers and collect similar factors
	CNumber Number;
	CNumber* pTermMultiplier;
	ElementsIterator it;
	for (it = pElements->begin(); it != SecondListStart; ) {
		if ( it->IsNumber() ) {
			Number += *it->pNumber;
			it = pElements->erase( it );
		}
		else { // look for similar terms
			ElementsIterator sub_it;
			if ( SecondListStart != pElements->end() )sub_it = SecondListStart;
			else {
				sub_it = it;
				++sub_it;
			}
			while ( sub_it != pElements->end() ) 
			{
				if ( it->IsSimilar( *sub_it ) ) 
				{
					pTermMultiplier = sub_it->GetMultiplier();
					it->Multiplier_Add( pTermMultiplier ? *pTermMultiplier : 1 );
					if ( sub_it == SecondListStart )
						SecondListStart = sub_it = pElements->erase( sub_it );
					else
						sub_it = pElements->erase( sub_it );
				}
				else
					++sub_it;
			}
			++it;
		}
	}
	if ( it != pElements->end() && it->IsNumber() ) { // process second list multiplier
		Number += *it->pNumber;
		pElements->erase( it );
	}
// Remove all factors with 0 multipliers and remove 1 multipliers from other factors
	for ( it = pElements->begin(); it != pElements->end(); ) {
		if ( (pTermMultiplier = it->GetMultiplier()) && *pTermMultiplier == 0 ) 
			it = pElements->erase( it );
		else {
			it->SimplifyMultiplier();
			++it;
		}
	}
// Put result free member != 0 to the head of the list
	Number.Simplify();
	if ( pElements->size() == 0 ) {
		delete pElements;
		Type = ctNumber;
		pNumber = new CNumber(Number);
	}
	else if ( Number != 0 )
		pElements->push_front( CMathTerm(Number) );
	else
		TryShrinkList();
}

void CMathExpression::SimplifyStruct()
{
	switch (Type) {
	case ctNumber:
		pNumber->Simplify();
		break;
	case ctElement: {
		pElement->SimplifyStruct();
		if ( pElement->IsNumber() ) 
		{
			pNumber = ReleasePointerAndDeleteOwner( pElement->pNumber, pElement );
			Type = ctNumber;
		}
		// Expr. = Expr.->T->F->E^1 => Expr = E
		else if ( pElement->IsElement() && pElement->pElement->IsExpression() && 
	  			  pElement->pElement->Degree == 1 )
			ExtractDataFromSubItem( pElement->pElement->pExpression );
		break;
	}
	case ctList: {
		// Chain subterms into one list: T->F->Expression->(T1,...,Tn) -> T1,...,Tn
		for ( ElementsIterator it = pElements->begin(); it != pElements->end(); ) {
			it->SimplifyStruct();
			if ( it->IsElement() && it->pElement->IsExpression() && 
	  			 it->pElement->Degree == 1  ) {
				if ( !it->pElement->pExpression->IsList() )
					throw CMathException_TypeIntegrityError();
				pElements->insert( it, it->pElement->pExpression->pElements->begin(), 
									   it->pElement->pExpression->pElements->end(), true);
				it = pElements->erase( it );
			}
			else
				++it;
		}
		CollectSimilarTerms();
		break;
	}
	}
}

CMathExpression& CMathExpression::Add(const CMathExpression& Expression, bool bOwnMode )
{
	if ( Type != ctList )
		ConvertToList();

	ElementsIterator SecondListStart;
	switch (Expression.Type) {
	case ctNumber:
		SecondListStart = pElements->push_back_( CMathTerm(*Expression.pNumber) );
		break;
	case ctElement:
		SecondListStart = pElements->push_back_( *Expression.pElement, bOwnMode );
		break;
	case ctList:
		SecondListStart = pElements->push_back_( Expression.pElements->begin(), Expression.pElements->end(), bOwnMode );
		break;
	}

	CollectSimilarTerms( SecondListStart );
	return *this;
}

CMathExpression& CMathExpression::Subtract(const CMathExpression& Expression, bool bOwnMode )
{
	if ( Type != ctList )
		ConvertToList();

	ElementsIterator SecondListStart;
	switch (Expression.Type) {
	case ctNumber:
		SecondListStart = pElements->push_back_( CMathTerm( -1 * *Expression.pNumber ) );
		break;
	case ctElement:
		SecondListStart = pElements->push_back_( *Expression.pElement, bOwnMode );
		pElements->back().Multiplier_Multiply(-1);
		break;
	case ctList: {
		ElementsIterator it = SecondListStart = pElements->push_back_( Expression.pElements->begin(), 
																	   Expression.pElements->end(), bOwnMode );
		for ( ; it != pElements->end(); ++it )
			it->Multiplier_Multiply(-1);
		break;
	}
	}

	CollectSimilarTerms( SecondListStart );
	return *this;
}

CMathExpression& CMathExpression::Multiply(CMathExpression& Expression, bool bOwnMode )
{
	if ( Type == ctNumber && Expression.Type != ctNumber ) {
		if ( *pNumber != 0 ) 
		{
			if ( bOwnMode ) 
			{
				Expression.Multiply(*this, true);
				SwapData( Expression );
			}
			else 
			{
				CMathExpression OtherExpression( Expression );
				OtherExpression.Multiply(*this, true);
				SwapData( OtherExpression );
			}
		}
	}
	else if ( Expression.Type == ctNumber ) 
	{
		if ( *Expression.pNumber == 0 ) 
		{
			DestroyData();
			Type = ctNumber;
			pNumber = new CNumber(0);
		}
		else {
			switch ( Type ) 
			{
			case ctNumber:
				*pNumber *= *Expression.pNumber;
				break;
			case ctElement:
				pElement->Multiplier_Multiply( *Expression.pNumber );
				break;
			case ctList: 
				{
				for ( ElementsIterator it = pElements->begin(); it != pElements->end(); ++it )
					it->Multiplier_Multiply( *Expression.pNumber );
				break;
			}
			}
			SimplifyStruct();
		}
	}
	else {
		CMathTerm* pNewTerm = new CMathTerm(ctList, true);
		CMathFactor addFactor(ftExpression, true);
		pNewTerm->pElements->move_back(addFactor);
		pNewTerm->pElements->back().pExpression->MoveData( *this );
		CMathFactor addFactor2(ftExpression, true);
		pNewTerm->pElements->move_back(addFactor2);
		if ( bOwnMode )
			pNewTerm->pElements->back().pExpression->MoveData( Expression );
		else
			pNewTerm->pElements->back().pExpression->CopyData( Expression );
		Type = ctElement;
		pElement = pNewTerm;

		SimplifyStruct();
	}
	return *this;
}

CMathExpression& CMathExpression::Divide(CMathExpression& Expression, bool bOwnMode )
{
	if ( Expression.Type == ctNumber ) 
	{
		switch ( Type ) 
		{
		case ctNumber:
			*pNumber /= *Expression.pNumber;
			break;
		case ctElement: {
			pElement->Multiplier_Divide( *Expression.pNumber );
			break;
		}
		case ctList: {
			for ( ElementsIterator it = pElements->begin(); it != pElements->end(); ++it )
				it->Multiplier_Divide( *Expression.pNumber );
			break;
		}
		}
	}
	else {
		CMathTerm* pNewTerm = new CMathTerm(ctList, true);
		CMathFactor addFactor(ftExpression, true);
		pNewTerm->pElements->move_back(addFactor);
		pNewTerm->pElements->back().pExpression->MoveData( *this );
		CMathFactor addFactor2(ftExpression, true, -1);
		pNewTerm->pElements->move_back(addFactor2);
		if ( bOwnMode )
			pNewTerm->pElements->back().pExpression->MoveData( Expression );
		else
			pNewTerm->pElements->back().pExpression->CopyData( Expression );
		Type = ctElement;
		pElement = pNewTerm;
	}
	SimplifyStruct();
	return *this;
}

void CMathExpression::Diff( const CMathParameter &Parameter )
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
		for ( ElementsIterator it = pElements->begin(); it != pElements->end(); ++it )
			it->Diff(Parameter);
		break;
	}
	}
	SimplifyStruct();
}

void CMathExpression::Power( int Degree )
{
	if ( Degree == 1 )
		return;
	else if ( Degree == 0 && Type != ctNumber ) {
		DestroyData();
		Type = ctNumber;
		pNumber = new CNumber(1);
		return;
	}

	switch ( Type ) 
	{
	case ctNumber:
		pNumber->Power(Degree);
		break;
	case ctElement:
		switch (pElement->Type) 
		{
		case ctNumber:
			//2DO:Check that it works correctly initially empty
			break;
		case ctElement:
			pElement->pElement->Degree *= Degree;
			break;
		case ctList:
			for ( CMathTerm::ElementsIterator it = pElement->pElements->begin(); it != pElement->pElements->end(); ++it )
				it->Power( Degree );
			break;
		}
		break;
	case ctList: {
	// Expr. --> E->T->F->Expr.^Degree
		CMathTerm* pNewTerm = new CMathTerm( new CMathFactor(ftExpression, true) );
		pNewTerm->pElement->pExpression->MoveData( *this );
		pNewTerm->pElement->Degree = Degree;
		Type = ctElement;
		pElement = pNewTerm;
		break;
	}
	}
}

void CMathExpression::InvertSign()
{
	switch ( Type ) 
	{
	case ctNumber:
		pNumber->Nominator = -pNumber->Nominator;
		break;
	case ctElement:
		pElement->Multiplier_Multiply(-1);
		break;
	case ctList: {
		for ( ElementsIterator it = pElements->begin(); it != pElements->end(); ++it )
			it->Multiplier_Multiply(-1);
		break;
	}
	}
}
