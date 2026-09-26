// MathFunctions.cpp: implementation of the math functions classes.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MathCalcMisc.h"
#include "MathExpressions.h"
#include "MathFunctions_impl.h"


namespace MathCalc {
namespace MathFunctionsStorage {
namespace {

	bool m_bInitialized = false;
	std::vector<MathString> m_FunctionsNames;
	std::map<MathString, int> m_IndicesMap;

	void RegisterFunction(const MathString& Name, int& StoredIndex )//;
	{
		m_FunctionsNames.push_back( Name );
		StoredIndex = (int)m_FunctionsNames.size() - 1;
		m_IndicesMap[Name] = StoredIndex;
	}

}}}

//////////////////////////////////////////////////////////////////////
// Static members

//int MathCalc::CMathFunc_exp::StorageIndex = -1;
//int MathCalc::CMathFunc_log::StorageIndex = -1;
//int MathCalc::CMathFunc_sin::StorageIndex = -1;
//int MathCalc::CMathFunc_cos::StorageIndex = -1;
//int MathCalc::CMathFunc_sqrt::StorageIndex = -1;

template<typename MathFunc> int MathCalc::CMathFuncStorage<MathFunc>::StorageIndex = -1;
//int MathCalc::CMathFuncStorage<MathCalc::CMathFunc_exp>::StorageIndex = -1;
//int MathCalc::CMathFuncStorage<MathCalc::CMathFunc_log>::StorageIndex = -1;
//int MathCalc::CMathFuncStorage<MathCalc::CMathFunc_sin>::StorageIndex = -1;
//int MathCalc::CMathFuncStorage<MathCalc::CMathFunc_cos>::StorageIndex = -1;
//int MathCalc::CMathFuncStorage<MathCalc::CMathFunc_sqrt>::StorageIndex = -1;

//////////////////////////////////////////////////////////////////////
// CMathFunctionsStorage

using namespace MathCalc;
using namespace MathCalc::MathFunctionsStorage;

void MathFunctionsStorage::Init()
{
	if ( MathFunctionsStorage::m_bInitialized )
		return;
	
	MathFunctionsStorage::RegisterFunction( CMathFunc_exp::GetName(), CMathFunc_exp::StorageIndex );
	MathFunctionsStorage::RegisterFunction( CMathFunc_log::GetName(), CMathFunc_log::StorageIndex );
	MathFunctionsStorage::RegisterFunction( CMathFunc_sin::GetName(), CMathFunc_sin::StorageIndex );
	MathFunctionsStorage::RegisterFunction( CMathFunc_cos::GetName(), CMathFunc_cos::StorageIndex );
	MathFunctionsStorage::RegisterFunction( CMathFunc_sqrt::GetName(), CMathFunc_sqrt::StorageIndex );

	MathFunctionsStorage::m_bInitialized = true;
}

//void MathFunctionsStorage::RegisterFunction( MathString& Name, int& StoredIndex )
//{
//	m_FunctionsNames.push_back( Name );
//	StoredIndex = m_FunctionsNames.size() - 1;
//	m_IndicesMap[Name] = StoredIndex;
//}

CMathFunction* MathFunctionsStorage::CreateFunction( int FunctionIndex )
{
	if ( FunctionIndex < 0 || (unsigned)FunctionIndex >= m_FunctionsNames.size() )
		return NULL;
	if ( FunctionIndex == CMathFunc_exp::GetStorageIndex() )
		return new CMathFunc_exp;
	else if ( FunctionIndex == CMathFunc_log::GetStorageIndex() )
		return new CMathFunc_log;
	else if ( FunctionIndex == CMathFunc_sin::GetStorageIndex() )
		return new CMathFunc_sin;
	else if ( FunctionIndex == CMathFunc_cos::GetStorageIndex() )
		return new CMathFunc_cos;
	else if ( FunctionIndex == CMathFunc_sqrt::GetStorageIndex() )
		return new CMathFunc_sqrt;
	return NULL;
}

CMathFunction* MathFunctionsStorage::CreateFunction( int FunctionIndex, bool bCreateItem )
{
	CMathFunction* pFunction = MathFunctionsStorage::CreateFunction(FunctionIndex);
	if ( pFunction )
		pFunction->pExpression = bCreateItem ? new CMathExpression(ctNumber) : NULL;
	return pFunction;
}

CMathFunction* MathFunctionsStorage::CreateFunction( int FunctionIndex, const CMathExpression& Expression )
{
	CMathFunction* pFunction = MathFunctionsStorage::CreateFunction(FunctionIndex);
	if ( pFunction )
		pFunction->pExpression = new CMathExpression(Expression);
	return pFunction;
}

CMathFunction* MathFunctionsStorage::CreateFunction( const MathString& Name, bool bCreateItem )
{
	int FunctionIndex;
	if ( MathFunctionsStorage::GetFunctionIndex( Name, FunctionIndex ) ) 
	{
		CMathFunction* pFunction = MathFunctionsStorage::CreateFunction(FunctionIndex);
		if ( pFunction )
			pFunction->pExpression = bCreateItem ? new CMathExpression(ctNumber) : NULL;
		return pFunction;
	}
	else
		return NULL;
}

CMathFunction* MathFunctionsStorage::CreateCopy( const CMathFunction& src )
{
	CMathFunction* pFunction = MathFunctionsStorage::CreateFunction(src.Index);
	if ( pFunction )
		pFunction->Copy(src);
	return pFunction;
}

bool MathFunctionsStorage::GetFunctionIndex( const MathString& FunctionName, int& FunctionIndex )
{
	std::map<MathString, int>::const_iterator pIndex = m_IndicesMap.find( FunctionName );
	if ( pIndex != m_IndicesMap.end() )
		FunctionIndex = pIndex->second;
	return pIndex != m_IndicesMap.end();
}

int MathFunctionsStorage::GetFunctionIndex( const MathString& FunctionName )
{
	std::map<MathString, int>::const_iterator pIndex = m_IndicesMap.find( FunctionName );
	if ( pIndex != m_IndicesMap.end() )
		return pIndex->second;
	else
		return -1;
}

MathString& MathFunctionsStorage::GetFunctionName( int FunctionIndex )
{
	return MathFunctionsStorage::m_FunctionsNames[FunctionIndex];
}

//////////////////////////////////////////////////////////////////////
// CMathFunction

CMathFunction::CMathFunction( const CMathFunction& src ) : Index(src.Index)
{
	Copy(src);
	IncreaseObjectsCounter();
}

CMathFunction::~CMathFunction()
{
	if ( pExpression != NULL ) {
		delete pExpression;
		pExpression = NULL;
	}
	DecreaseObjectsCounter();
}

void CMathFunction::Copy( const CMathFunction& src )
{
	if ( this != &src ) {
		if (Index != src.Index)
			throw CMathException_FunctionCopyError();
		pExpression = (src.pExpression ? new CMathExpression(*src.pExpression) : NULL);
	}
}

inline bool CMathFunction::IsEqual(const CMathFunction& Operand) const
{
	return Index == Operand.Index && pExpression->IsEqual(*Operand.pExpression);
}

void CMathFunction::Serialize( CMathSerializer& ar ) 
{
	if ( ar.IsLoading() )
		pExpression = new CMathExpression(ctNumber);
	pExpression->Serialize( ar );
}

inline CMathExpression* CMathFunction::Diff( const CMathParameter& Parameter )
{ // (F(expr))[param] = expr[param] * ?
	CMathExpression* pDiffExpression = new CMathExpression( new CMathTerm(ctList, true) );
	CMathFactor addFactor(*pExpression);
	pDiffExpression->pElement->pElements->move_back(addFactor);
	pDiffExpression->pElement->pElements->back().Diff(Parameter);
	return pDiffExpression;
}


//////////////////////////////////////////////////////////////////////
// Specific functions
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// SimplifyStruct

CMathExpression* CMathFunc_exp::SimplifyStruct()
{
// exp(ln(Expr)) --> Expr
	if ( pExpression != NULL && pExpression->IsElement() && 
		 pExpression->pElement->IsElement() && pExpression->pElement->pElement->Degree == 1 &&
		 pExpression->pElement->pElement->IsFunction() &&
		 pExpression->pElement->pElement->pFunction->Index == CMathFunc_log::GetStorageIndex() ) { 
		CMathExpression* pResultExpression = pExpression->pElement->pElement->pFunction->pExpression;
		pExpression->pElement->pElement->pFunction->pExpression = NULL;
		return pResultExpression;
	}
	else
		return NULL;
}

CMathExpression* CMathFunc_log::SimplifyStruct()
{
// ln(exp(Expr)) --> Expr
	if ( pExpression != NULL && pExpression->Type == ctElement && 
		 pExpression->pElement->Type == ctElement && pExpression->pElement->pElement->Degree == 1 && 
		 pExpression->pElement->pElement->Type == ftFunction &&
		 pExpression->pElement->pElement->pFunction->Index == CMathFunc_exp::GetStorageIndex() ) { 
		CMathExpression* pResultExpression = pExpression->pElement->pElement->pFunction->pExpression;
		pExpression->pElement->pElement->pFunction->pExpression = NULL;
		return pResultExpression;
	}
	else
		return NULL;
}

//////////////////////////////////////////////////////////////////////
// Diff

CMathExpression* CMathFunc_exp::Diff( const CMathParameter& Parameter )
{
	CMathExpression* pDiffExpression = CMathFunction::Diff(Parameter);
	CMathFactor addFactor(MathFunctionsStorage::CreateFunction(Index));
	pDiffExpression->pElement->pElements->move_back(addFactor);
	pDiffExpression->pElement->pElements->back().pFunction->pExpression = pExpression;
	pDiffExpression->SimplifyStruct();
	pExpression = NULL;
	return pDiffExpression;
}

CMathExpression* CMathFunc_log::Diff( const CMathParameter& Parameter )
{
	CMathExpression* pDiffExpression = CMathFunction::Diff(Parameter);
	CMathFactor addFactor( pExpression, -1 );
	pDiffExpression->pElement->pElements->move_back(addFactor);
	pDiffExpression->SimplifyStruct();
	pExpression = NULL;
	return pDiffExpression;
}

CMathExpression* CMathFunc_sin::Diff( const CMathParameter& Parameter )
{
	CMathExpression* pDiffExpression = CMathFunction::Diff(Parameter);
	CMathFactor addFactor(MathFunctionsStorage::CreateFunction(CMathFunc_cos::GetStorageIndex()));
	pDiffExpression->pElement->pElements->move_back(addFactor);
	pDiffExpression->pElement->pElements->back().pFunction->pExpression = pExpression;
	pDiffExpression->SimplifyStruct();
	pExpression = NULL;
	return pDiffExpression;
}

CMathExpression* CMathFunc_cos::Diff( const CMathParameter& Parameter )
{
	CMathExpression* pDiffExpression = CMathFunction::Diff(Parameter);
	pDiffExpression->pElement->pElements->push_front( CMathFactor( CNumber(-1) ) );
	CMathFactor addFactor( MathFunctionsStorage::CreateFunction(CMathFunc_sin::GetStorageIndex()));
	pDiffExpression->pElement->pElements->move_back(addFactor);
	pDiffExpression->pElement->pElements->back().pFunction->pExpression = pExpression;
	pDiffExpression->SimplifyStruct();
	pExpression = NULL;
	return pDiffExpression;
}

CMathExpression* CMathFunc_sqrt::Diff( const CMathParameter& Parameter )
{
	CMathExpression* pDiffExpression = CMathFunction::Diff(Parameter);
	pDiffExpression->pElement->pElements->push_front( CMathFactor( CNumber(1, 2) ) );
	CMathFactor addFactor( MathFunctionsStorage::CreateFunction(CMathFunc_sqrt::GetStorageIndex()), -1);
	pDiffExpression->pElement->pElements->move_back(addFactor);
	pDiffExpression->pElement->pElements->back().pFunction->pExpression = pExpression;
	pDiffExpression->SimplifyStruct();
	pExpression = NULL;
	return pDiffExpression;
}
