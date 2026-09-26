#ifndef MATHFUNCTIONS_IMPL_H
#define MATHFUNCTIONS_IMPL_H

// MathFunctions_impl.h: implementation details
//
//////////////////////////////////////////////////////////////////////
#include "MathFunctions.h"

namespace MathCalc {

//////////////////////////////////////////////////////////////////////
// Specific functions classes

template< class ThisClass >
class CMathFuncStorage {
	friend void MathFunctionsStorage::Init();
protected:
	static int StorageIndex;
public:
	static int GetStorageIndex() { return ThisClass::StorageIndex; }
};

class CMathFunc_exp : public CMathFunction, public CMathFuncStorage<CMathFunc_exp>
{
public:
	static MathString GetName() { return _T("exp"); }
	virtual CMathExpression* SimplifyStruct();
	virtual CMathExpression* Diff( const CMathParameter& Parameter );
	virtual double Eval() const { return exp( pExpression->Eval().Eval() ); }
	CMathFunc_exp() : CMathFunction( CMathFunc_exp::StorageIndex ) {};
};

class CMathFunc_log : public CMathFunction, public CMathFuncStorage<CMathFunc_log>
{
public:
	static MathString GetName() { return _T("ln"); }
	virtual CMathExpression* SimplifyStruct();
	virtual CMathExpression* Diff( const CMathParameter& Parameter );
	virtual double Eval() const { return log( pExpression->Eval().Eval() ); }
	CMathFunc_log() : CMathFunction( CMathFunc_log::StorageIndex ) {};
};

class CMathFunc_sin : public CMathFunction, public CMathFuncStorage<CMathFunc_sin> 
{
public:
	static MathString GetName() { return _T("sin"); }
	virtual CMathExpression* SimplifyStruct() { return NULL; }
	virtual CMathExpression* Diff( const CMathParameter& Parameter );
	virtual double Eval() const { return sin( pExpression->Eval().Eval() ); }
	CMathFunc_sin() : CMathFunction( CMathFunc_sin::StorageIndex ) {};
};

class CMathFunc_cos : public CMathFunction, public CMathFuncStorage<CMathFunc_cos> 
{
public:
	static MathString GetName() { return _T("cos"); }
	virtual CMathExpression* SimplifyStruct() { return NULL; }
	virtual CMathExpression* Diff( const CMathParameter& Parameter );
	virtual double Eval() const { return cos( pExpression->Eval().Eval() ); }
	CMathFunc_cos() : CMathFunction( CMathFunc_cos::StorageIndex ) {};
};

class CMathFunc_sqrt : public CMathFunction, public CMathFuncStorage<CMathFunc_sqrt> 
{
public:
	static MathString GetName() { return _T("sqrt"); }
	virtual CMathExpression* SimplifyStruct() { return NULL; }
	virtual CMathExpression* Diff( const CMathParameter& Parameter );
	virtual double Eval() const { return sqrt( pExpression->Eval().Eval() ); }
	CMathFunc_sqrt() : CMathFunction( CMathFunc_sqrt::StorageIndex ) {};
};

} // namespace MathCalc
#endif //MATHFUNCTIONS_IMPL_H
