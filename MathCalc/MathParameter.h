#ifndef MATHPARAMETER_H
#define MATHPARAMETER_H


#include "Expressions.h"
#include "MathTemplates.h"
namespace MathCalc 
{
class CMathParameter
{
public:
	int Index;

	//UINT actually is unsigned int but 
	CMathParameter( int ParameterIndex = -1 ) : Index(ParameterIndex) {}; // -1 means uninitialized parameter
	explicit CMathParameter( const MathString& Name );
	//-->Sergey
	//	CMathParameter(const CMathParameter& param)
	//	{
	//		Index=param.Index;
	//	}
	//<--Sergey
	// Operations
	MathString ToString() const;
	CNumber Eval() const { return MathContext::GetParameterValue( Index ); }
	bool IsEqual( const CMathParameter& Operand ) const { return Index == Operand.Index; }
	void SetValue( double Value ) const { MathContext::SetParameterValue( Index, Value ); }
	double GetValue() const { return MathContext::GetParameterValue( Index ); }
	void Serialize( CMathSerializer& ar );
};
}//namespace MathCalc
#endif //MATHPARAMETER_H
