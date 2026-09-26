// Expressions.h: interface classes and functions for CMath... classes.
//
//////////////////////////////////////////////////////////////////////

#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H
#include "MathCalcMisc.h"

namespace MathCalc {

//////////////////////////////////////////////////////////////////////
// CMathParameter wrapper class

class CMathParameter;

class MATHCALC_API CParameter 
{
	friend class CExpression;
	CMathParameter* m_pParameter;
public:
	CParameter(); // Constructs invalid parameter
	CParameter( const CParameter& src );
	explicit CParameter( const MathString& ParameterName );
	CParameter& operator=( const CParameter& src );
	~CParameter(void);
	
// Compares two parameters
	bool operator==( const CParameter& Operand ) const;
// Returns string representation of this parameter
	MathString ToString() const;
// Returns parameter value from MathContext
	double Eval() const;
// Sets parameter value to MathContext
	void SetValue( double Value ) const;
// Gets parameter value from MathContext
	double GetValue() const;
// Saves/Loads parameter to/from the given archive
	void Serialize( CMathSerializer& ar );
};

//////////////////////////////////////////////////////////////////////
// CMathExpression wrapper class

class CMathExpression;

class MATHCALC_API CExpression
{
	CMathExpression* m_pExpression;
public:
	CExpression( double Number = 0 );
	CExpression( const CExpression& src );
	explicit CExpression( const CParameter& Parameter );
	~CExpression(void);

// Just copies data from another expression
	CExpression& operator=( const CExpression& src );
// Exchanges internal representations
	void Swap( CExpression& expr ) // swaps data
		{ std::swap( m_pExpression, expr.m_pExpression ); }

// Returns string representation of this expression
	MathString ToString( int nOutputPrecision = 2 ) const;
// Returns expression value using parameters values in MathContext
	double Eval() const;
// Checks if expression is a number
	bool IsNumber() const;

// Constructs expression from a given string
	bool ParseString( const MathString& pszExpression );
// Saves/Loads expression to/from the given archive
	void Serialize( CMathSerializer& ar );

// Expression --> Expression^Degree
	void Power( int Degree );
// Expression --> -Expression
	void InvertSign();
// Expression --> FunctionName(Expression)
	void ApplyFunction( const MathString& FunctionName ) /*throw( CMathException_InvalidFunctionName )*/;
// Expression --> Diff(Expression, Parameter)
	void Diff( const CParameter& Parameter );
// Expression --> Diff@Order(Expression, Parameter)
	void Diff( const CParameter& Parameter, int Order );
// Returns Diff@Order(Expression, Parameter)
	CExpression operator() ( const CParameter& Parameter, int Order = 1 ) const 
	{
		CExpression result(*this);
		result.Diff( Parameter, Order );
		return result;
	}

// Arithmetic operations; 
// bOwnMode signals that the given expression CAN be modified (fast calculation mode)
// If bOwnMode = false then the given expression will NOT be modified (low, default)
// You shouldn't use the given expression after operation is called with bOwnMode = true
	//-->Sergey
	//Add const
	CExpression& Add(const CExpression& E, bool bOwnMode = false );
	CExpression& Subtract(const CExpression& E, bool bOwnMode = false );
	//<--Sergey
	CExpression& Multiply( CExpression& E, bool bOwnMode = false );
	CExpression& Divide( CExpression& E, bool bOwnMode = false );

	//-->Sergey
	//delete const_cast
	//CExpression& operator+=( const CExpression& E ) 
	//	{ return Add( const_cast<CExpression&>(E) ); }
	//CExpression& operator-=( const CExpression& E ) 
	//	{ return Subtract( const_cast<CExpression&>(E) ); }
	CExpression& operator+=( const CExpression& E ) 
		{ return Add( E ); }
	CExpression& operator-=( const CExpression& E ) 
		{ return Subtract( E ); }

	//<--Sergey
	CExpression& operator*=( const CExpression& E ) 
		{ return Multiply( const_cast<CExpression&>(E) ); }
	CExpression& operator/=( const CExpression& E ) 
		{ return Divide( const_cast<CExpression&>(E) ); }
};

MATHCALC_API CExpression operator+( const CExpression&, const CExpression& );
MATHCALC_API CExpression operator-( const CExpression&, const CExpression& );
MATHCALC_API CExpression operator*( const CExpression&, const CExpression& );
MATHCALC_API CExpression operator/( const CExpression&, const CExpression& );
MATHCALC_API CExpression operator^( const CExpression&, int Degree );

} // namespace MathCalc
#endif //EXPRESSIONS_H
