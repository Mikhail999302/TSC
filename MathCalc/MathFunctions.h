#ifndef MATHFUNCTIONS_H
#define MATHFUNCTIONS_H

// MathFunctions.h: functions classes declaration
//
//////////////////////////////////////////////////////////////////////
namespace MathCalc {

//////////////////////////////////////////////////////////////////////
// External dependencies

class CMathExpression;

//////////////////////////////////////////////////////////////////////
// Functions storage

class CMathFunction;
namespace MathFunctionsStorage {
	void Init();
	CMathFunction* CreateFunction( int FunctionIndex );
	CMathFunction* CreateFunction( int FunctionIndex, bool bCreateItem );
	CMathFunction* CreateFunction( int FunctionIndex, const CMathExpression& Expression );
	CMathFunction* CreateFunction( const MathString& Name, bool bCreateItem );
	CMathFunction* CreateCopy( const CMathFunction& src );
	bool GetFunctionIndex( const MathString& FunctionName, int& FunctionIndex );
	int GetFunctionIndex( const MathString& FunctionName );
	MathString& GetFunctionName( int FunctionIndex );
}

//////////////////////////////////////////////////////////////////////
// Function

class CMathFunction 
{
	friend CMathFunction* MathFunctionsStorage::CreateCopy( const CMathFunction& );
protected:
	virtual void Copy( const CMathFunction& src );
public:
	const int Index;
	CMathExpression* pExpression;

	virtual MathString ToString( int nOutputPrecision ) const;
	virtual bool IsEqual( const CMathFunction& Operand ) const;
	virtual double Eval() const = 0;
	virtual CMathExpression* SimplifyStruct() = 0;
	virtual CMathExpression* Diff( const CMathParameter& Parameter );
	virtual void Serialize( CMathSerializer& ar );

	CMathFunction( int _StorageIndex ) : Index(_StorageIndex), pExpression(NULL) { IncreaseObjectsCounter(); }
	CMathFunction( const CMathFunction& src );
	virtual ~CMathFunction();
};

} // namespace MathCalc
#endif //MATHFUNCTIONS_H
